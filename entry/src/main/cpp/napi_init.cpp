#include "napi/native_api.h"
#include <EGL/egl.h>
#include <GLES3/gl32.h>
#include <assert.h>
#include <fcntl.h>
#include <map>
#include <native_window/external_window.h>
#include <poll.h>
#include <pty.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "hilog/log.h"
#undef LOG_TAG
#define LOG_TAG "testTag"

static int fd = -1;

// maintain terminal status
struct term_char {
    char ch = ' ';
};

static std::vector<std::vector<term_char>> terminal;
static int row = 0;
static int col = 0;
static int escape_state = 0;
static std::string escape_buffer;

static napi_value Run(napi_env env, napi_callback_info info) {
    if (fd != -1) {
        return nullptr;
    }

    struct winsize ws = {};
    ws.ws_col = 80;
    ws.ws_row = 24;
    int pid = forkpty(&fd, nullptr, nullptr, &ws);
    if (!pid) {
        execl("/bin/sh", "/bin/sh", nullptr);
    }

    assert(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) == 0);
    return nullptr;
}

static napi_value Read(napi_env env, napi_callback_info info) {
    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLIN;
    int res = poll(fds, 1, 5000);
    static char buffer[16384];
    if (res > 0) {
        ssize_t r = read(fd, buffer, sizeof(buffer) - 1);
        if (r > 0) {
            // parse output
            for (int i = 0; i < r; i++) {
                while (row >= terminal.size()) {
                    terminal.push_back(std::vector<term_char>());
                }
                while (col >= terminal[row].size()) {
                    terminal[row].push_back(term_char());
                }

                if (escape_state != 0) {
                    if (buffer[i] == 91) {
                        // opening bracket, CSI
                        escape_state = 2;
                    } else {
                        // unknown
                        escape_state = 0;
                    }
                } else {
                    if (buffer[i] >= ' ' && buffer[i] < 127) {
                        // printable
                        terminal[row][col].ch = buffer[i];
                        col++;
                    } else if (buffer[i] == '\r') {
                        col = 0;
                    } else if (buffer[i] == '\n') {
                        row += 1;
                    } else if (buffer[i] == '\b') {
                        terminal[row][col].ch = ' ';
                        col -= 1;
                    } else if (buffer[i] == 0x1b) {
                        escape_buffer = "";
                        escape_state = 1;
                    }
                }
            }

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

static napi_value Send(napi_env env, napi_callback_info info) {
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


// https://learnopengl.com/In-Practice/Text-Rendering
struct ivec2 {
    int x;
    int y;

    ivec2(int x, int y) {
        this->x = x;
        this->y = y;
    }
    ivec2() { this->x = this->y = 0; }
};

struct character {
    unsigned int textureID; // ID handle of the glyph texture
    ivec2 size;             // Size of glyph
    ivec2 bearing;          // Offset from baseline to left/top of glyph
    unsigned int advance;   // Offset to advance to next glyph
};

std::map<char, struct character> characters;

// load font
static void LoadFont() {
    FT_Library ft;
    assert(FT_Init_FreeType(&ft) == 0);

    FT_Face face;
    assert(FT_New_Face(ft, "/data/storage/el2/base/haps/entry/files/Inconsolata-Regular.ttf", 0, &face) == 0);
    FT_Set_Pixel_Sizes(face, 0, 48);
    assert(FT_Load_Char(face, 'X', FT_LOAD_RENDER) == 0);

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++) {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            continue;
        }

        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED,
                     GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // now store character for later use
        character character = {texture, ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                               ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                               (unsigned int)face->glyph->advance.x};
        characters.insert(std::pair<char, struct character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

static EGLDisplay egl_display;
static EGLSurface egl_surface;
static GLuint program_id;
static GLuint vertex_array;
static GLuint vertex_buffer;

static void Draw() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (int i = 0; i < terminal.size(); i++) {
        float scale = 0.005;
        float x = -1.0;
        float y = -i * 0.2;

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(vertex_array);
        for (auto c : terminal[i]) {
            character ch = characters[c.ch];
            float xpos = x + ch.bearing.x * scale;
            float ypos = y - (ch.size.y - ch.bearing.y) * scale;
            float w = ch.size.x * scale;
            float h = ch.size.y * scale;

            // An array of 3 vectors which represents 3 vertices
            GLfloat g_vertex_buffer_data[24] = {xpos,     ypos + h, 0.0f, 0.0f, xpos,     ypos,     0.0f, 1.0f,
                                                xpos + w, ypos,     1.0f, 1.0f, xpos,     ypos + h, 0.0f, 0.0f,
                                                xpos + w, ypos,     1.0f, 1.0f, xpos + w, ypos + h, 1.0f, 0.0f};
            glBindTexture(GL_TEXTURE_2D, ch.textureID);
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_vertex_buffer_data), g_vertex_buffer_data);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            x += ((float)ch.advance / 64) * scale;
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glFlush();
    glFinish();
    eglSwapBuffers(egl_display, egl_surface);
}

static napi_value CreateSurface(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    int64_t surface_id = 0;
    bool lossless = true;
    assert(napi_get_value_bigint_int64(env, args[0], &surface_id, &lossless) == napi_ok);

    // create windows and display
    OHNativeWindow *native_window;
    OH_NativeWindow_CreateNativeWindowFromSurfaceId(surface_id, &native_window);
    assert(native_window);
    EGLNativeWindowType egl_window = (EGLNativeWindowType)native_window;
    egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(egl_display != EGL_NO_DISPLAY);

    // initialize egl
    EGLint major_version;
    EGLint minor_version;
    assert(eglInitialize(egl_display, &major_version, &minor_version) == EGL_TRUE);

    const EGLint attrib[] = {EGL_SURFACE_TYPE,
                             EGL_WINDOW_BIT,
                             EGL_RENDERABLE_TYPE,
                             EGL_OPENGL_ES2_BIT,
                             EGL_RED_SIZE,
                             8,
                             EGL_GREEN_SIZE,
                             8,
                             EGL_BLUE_SIZE,
                             8,
                             EGL_ALPHA_SIZE,
                             8,
                             EGL_DEPTH_SIZE,
                             24,
                             EGL_STENCIL_SIZE,
                             8,
                             EGL_SAMPLE_BUFFERS,
                             1,
                             EGL_SAMPLES,
                             4, // Request 4 samples for multisampling
                             EGL_NONE};

    const EGLint max_config_size = 1;
    EGLint num_configs;
    EGLConfig egl_config;
    assert(eglChooseConfig(egl_display, attrib, &egl_config, max_config_size, &num_configs) == EGL_TRUE);

    egl_surface = eglCreateWindowSurface(egl_display, egl_config, egl_window, NULL);

    EGLint context_attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    EGLContext egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, context_attributes);

    eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

    // build vertex and fragment shader
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    char const *vertex_source = "#version 320 es\n"
                                "\n"
                                "in vec4 vertex;\n"
                                "out vec2 texCoords;\n"
                                "void main() {\n"
                                "  gl_Position.xy = vertex.xy;\n"
                                "  gl_Position.z = 0.0;\n"
                                "  gl_Position.w = 1.0;\n"
                                "  texCoords = vertex.zw;\n"
                                "}";
    glShaderSource(vertex_shader_id, 1, &vertex_source, NULL);
    glCompileShader(vertex_shader_id);

    int info_log_length;
    glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        std::vector<char> vertex_shader_error_message(info_log_length + 1);
        glGetShaderInfoLog(vertex_shader_id, info_log_length, NULL, &vertex_shader_error_message[0]);
        OH_LOG_INFO(LOG_APP, "Failed to build vertex shader: %{public}s", &vertex_shader_error_message[0]);
    }

    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    char const *fragment_source = "#version 320 es\n"
                                  "\n"
                                  "precision lowp float;\n"
                                  "in vec2 texCoords;\n"
                                  "out vec4 color;\n"
                                  "uniform sampler2D text;\n"
                                  "uniform vec3 textColor;\n"
                                  "void main() {\n"
                                  "  vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, texCoords).r);\n"
                                  "  color = vec4(textColor, 1.0) * sampled;\n"
                                  "}";
    glShaderSource(fragment_shader_id, 1, &fragment_source, NULL);
    glCompileShader(fragment_shader_id);

    glGetShaderiv(fragment_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        std::vector<char> fragment_shader_error_message(info_log_length + 1);
        glGetShaderInfoLog(fragment_shader_id, info_log_length, NULL, &fragment_shader_error_message[0]);
        OH_LOG_INFO(LOG_APP, "Failed to build fragment shader: %{public}s", &fragment_shader_error_message[0]);
    }

    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);

    glUseProgram(program_id);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // load font from ttf
    LoadFont();

    // create buffers for drawing
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,                 // attribute 0
                          4,                 // size
                          GL_FLOAT,          // type
                          GL_FALSE,          // normalized?
                          4 * sizeof(float), // stride
                          (void *)0          // array buffer offset
    );
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Draw();

    return nullptr;
}

static napi_value Redraw(napi_env env, napi_callback_info info) {
    Draw();
    return nullptr;
}

static napi_value DestroySurface(napi_env env, napi_callback_info info) { return nullptr; }

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        {"run", nullptr, Run, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"read", nullptr, Read, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"send", nullptr, Send, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"redraw", nullptr, Redraw, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"createSurface", nullptr, CreateSurface, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"destroySurface", nullptr, DestroySurface, nullptr, nullptr, nullptr, napi_default, nullptr},
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
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void) { napi_module_register(&demoModule); }
