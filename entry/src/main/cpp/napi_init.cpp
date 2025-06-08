#include "napi/native_api.h"
#include <string>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <pty.h>
#include <poll.h>
#include <EGL/egl.h>
#include <native_window/external_window.h>
#include <GLES3/gl32.h>

static int fd = -1;

static napi_value Run(napi_env env, napi_callback_info info)
{
    if (fd != -1) {
        return nullptr;
    }

    struct winsize ws = {};
    ws.ws_col = 80;
    ws.ws_row = 24;
    int pid = forkpty(&fd, nullptr, nullptr, &ws);
    if(!pid)
    {
        execl("/bin/sh", "/bin/sh", nullptr);
    }

    assert(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) == 0);
    return nullptr;
}

static napi_value Read(napi_env env, napi_callback_info info)
{
    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLIN;
    int res = poll(fds, 1, 5000);
    static char buffer[16384];
    if (res > 0) {
        ssize_t r = read(fd, buffer, sizeof(buffer)-1);
        if (r > 0) {
            buffer[r] = '\0';
            napi_value ret;
            void *data = nullptr;
            assert(napi_create_arraybuffer(env, r, &data, &ret) == napi_ok);
            memcpy(data, buffer, r);
            return ret;
        } else if (r == -1 && errno != EAGAIN) {
            // error
            return nullptr;
        }
    }

    // empty: nothing available yet
    napi_value ret;
    void *data = nullptr;
    assert(napi_create_arraybuffer(env, 0, &data, &ret) == napi_ok);
    return ret;
}

static napi_value Send(napi_env env, napi_callback_info info)
{
    if (fd == -1) {
        return nullptr;
    }

    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    void *data;
    size_t length;
    assert(napi_get_arraybuffer_info(env, args[0], &data, &length) == napi_ok);
    int written = 0;
    while (written < length) {
        int size = write(fd, (uint8_t *)data + written, length - written);
        assert(size >= 0);
        written += size;
    }
    return nullptr;
}

static napi_value CreateSurface(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    int64_t surface_id = 0;
    bool lossless = true;
    assert(napi_get_value_bigint_int64(env, args[0], &surface_id, &lossless) == napi_ok);

    OHNativeWindow *native_window;
    OH_NativeWindow_CreateNativeWindowFromSurfaceId(surface_id, &native_window);
    assert(native_window);
    EGLNativeWindowType eglWindow_ = (EGLNativeWindowType)native_window;
    EGLDisplay eglDisplay_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(eglDisplay_ != EGL_NO_DISPLAY);

    EGLint majorVersion;
    EGLint minorVersion;
    assert(eglInitialize(eglDisplay_, &majorVersion, &minorVersion) == EGL_TRUE);

    const EGLint attrib[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_SAMPLE_BUFFERS, 1,
        EGL_SAMPLES, 4, // Request 4 samples for multisampling
        EGL_NONE
    };

    const EGLint maxConfigSize = 1;
    EGLint numConfigs;
    EGLConfig eglConfig_;
    assert(eglChooseConfig(eglDisplay_, attrib, &eglConfig_, maxConfigSize, &numConfigs) == EGL_TRUE);

    EGLSurface eglSurface_ = eglCreateWindowSurface(eglDisplay_, eglConfig_, eglWindow_, NULL);

    EGLint contextAttributes[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    EGLContext eglContext_ = eglCreateContext(eglDisplay_, eglConfig_, EGL_NO_CONTEXT, contextAttributes);

    eglMakeCurrent(eglDisplay_, eglSurface_, eglSurface_, eglContext_);

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
    glFinish();
    eglSwapBuffers(eglDisplay_, eglSurface_);

    return nullptr;
}

static napi_value DestroySurface(napi_env env, napi_callback_info info)
{
    return nullptr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        { "run", nullptr, Run, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "read", nullptr, Read, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "send", nullptr, Send, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "createSurface", nullptr, CreateSurface, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "destroySurface", nullptr, DestroySurface, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "entry",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void)
{
    napi_module_register(&demoModule);
}
