#include "napi/native_api.h"
#include <EGL/egl.h>
#include <GLES3/gl32.h>
#include <assert.h>
#include <deque>
#include <fcntl.h>
#include <map>
#include <native_window/external_window.h>
#include <poll.h>
#include <pty.h>
#include <stdio.h>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "hilog/log.h"
#undef LOG_TAG
#define LOG_TAG "testTag"

static int fd = -1;

enum weight {
    regular = 0,
    bold = 1,
    NUM_WEIGHT,
};

// maintain terminal status
struct style {
    weight weight = regular;
    float red = 0.0;
    float green = 0.0;
    float blue = 0.0;
};
struct term_char {
    char ch = ' ';
    style style;
};

static int MAX_HISTORY_LINES = 5000;
static std::deque<std::vector<term_char>> history;
static std::vector<std::vector<term_char>> terminal;
static int row = 0;
static int col = 0;
enum escape_states {
    state_idle,
    state_esc,
    state_csi,
    state_osc,
};
static escape_states escape_state = state_idle;
static std::string escape_buffer;
static style current_style;
static int width = 0;
static int height = 0;
static bool show_cursor = true;
static GLint surface_location = -1;
static int font_height = 48;
static int font_width = 24;
static int baseline_height = 10;
static int term_col = 80;
static int term_row = 24;
static float scroll_offset = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

extern "C" int mkdir(const char *pathname, mode_t mode);
static napi_value Run(napi_env env, napi_callback_info info) {
    if (fd != -1) {
        return nullptr;
    }

    terminal.resize(term_row);
    for (int i = 0; i < term_row; i++) {
        terminal[i].resize(term_col);
    }

    struct winsize ws = {};
    ws.ws_col = term_col;
    ws.ws_row = term_row;

    int pid = forkpty(&fd, nullptr, nullptr, &ws);
    if (!pid) {
        // override HOME to /storage/Users/currentUser since it is writable
        const char *home = "/storage/Users/currentUser";
        setenv("HOME", home, 1);
        setenv("PWD", home, 1);
        chdir(home);
        execl("/data/app/bin/bash", "/data/app/bin/bash", nullptr);
    }

    int res = fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    assert(res == 0);
    return nullptr;
}

std::vector<std::string> splitString(const std::string &str, const std::string &delimiter) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    result.push_back(str.substr(start));
    return result;
}

static napi_value Send(napi_env env, napi_callback_info info) {
    if (fd == -1) {
        return nullptr;
    }

    // reset scroll offset to bottom
    scroll_offset = 0.0;

    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    void *data;
    size_t length;
    napi_status ret = napi_get_arraybuffer_info(env, args[0], &data, &length);
    assert(ret == napi_ok);
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
    // location within the large texture
    float left;
    float right;
    float top;
    float bottom;
};

// record info for each character
const int MAX_CHAR = 128;
std::array<std::array<struct character, NUM_WEIGHT>, MAX_CHAR> characters;

// id of texture for glyphs
static GLuint texture_id;

// load font
// texture contains all glyphs of all weights:
// each glyph: font_width * font_height
//    0.0   1/128                1.0
// 0.0 +------+------+     +------+
//     | 0x00 | 0x01 | ... | 0x7f | regular
// 0.5 +------+------+     +------+
//     | 0x00 | 0x01 | ... | 0x7f | bold
// 1.0 +------+------+     +------+
static void LoadFont() {
    FT_Library ft;
    FT_Error err = FT_Init_FreeType(&ft);
    assert(err == 0);

    std::vector<std::pair<const char *, weight>> fonts = {
        {"/data/storage/el2/base/haps/entry/files/Inconsolata-Regular.ttf", weight::regular},
        {"/data/storage/el2/base/haps/entry/files/Inconsolata-Bold.ttf", weight::bold},
    };

    // save glyph for all characters of all weights
    // only one channel
    std::vector<uint8_t> bitmap;
    int row_stride = font_width * MAX_CHAR;
    int weight_stride = font_width * font_height * MAX_CHAR;
    bitmap.resize(font_width * font_height * MAX_CHAR * NUM_WEIGHT);

    for (auto pair : fonts) {
        const char *font = pair.first;
        weight weight = pair.second;

        FT_Face face;
        err = FT_New_Face(ft, font, 0, &face);
        assert(err == 0);
        FT_Set_Pixel_Sizes(face, 0, font_height);

        for (unsigned char c = 0; c < MAX_CHAR; c++) {
            // load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                continue;
            }

            OH_LOG_INFO(LOG_APP,
                        "Char: %{public}c Size: %{public}d %{public}d Glyph: %{public}d %{public}d Left: %{public}d "
                        "Top: %{public}d "
                        "Advance: %{public}ld",
                        c, font_width, font_height, face->glyph->bitmap.width, face->glyph->bitmap.rows,
                        face->glyph->bitmap_left, face->glyph->bitmap_top, face->glyph->advance.x);

            // copy to bitmap font_width * font_height
            for (int i = 0; i < face->glyph->bitmap.rows; i++) {
                for (int j = 0; j < face->glyph->bitmap.width; j++) {
                    // compute offset within a glyph
                    // x goes rightward, y goes downward:
                    // set (0, 0) to origin,
                    // then the top left corner of glyph is at (-bitmap_top, bitmap_left)
                    // now move origin to (font_height - baseline_height, 0)
                    // top left corner becomes (font_height - baseline_height - bitmap_top, bitmap_left)
                    int xoff = font_height - baseline_height - face->glyph->bitmap_top;
                    int yoff = face->glyph->bitmap_left;
                    assert(i + xoff >= 0 && i + xoff < font_height);
                    assert(j + yoff >= 0 && j + yoff < font_width);

                    // compute offset in the large texture
                    int off = weight * weight_stride + c * font_width;
                    bitmap[(i + xoff) * row_stride + (j + yoff) + off] =
                        face->glyph->bitmap.buffer[i * face->glyph->bitmap.width + j];
                }
            }

            // compute location within the texture
            character character = {
                .left = (float)c / MAX_CHAR,
                .right = (float)(c + 1) / MAX_CHAR,
                .top = (float)weight / NUM_WEIGHT,
                .bottom = (float)(weight + 1) / NUM_WEIGHT,
            };
            characters[c][weight] = character;
        }

        FT_Done_Face(face);
    }

    FT_Done_FreeType(ft);

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // generate texture
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, row_stride, font_height * NUM_WEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE,
                 bitmap.data());

    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

static EGLDisplay egl_display;
static EGLSurface egl_surface;
static EGLContext egl_context;
static GLuint program_id;
static GLuint vertex_array;
// vec4 vertex
static GLuint vertex_buffer;
// vec3 textColor
static GLuint text_color_buffer;
// vec3 backGroundColor
static GLuint background_color_buffer;

static void Draw() {
    // clear buffer
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update surface size
    pthread_mutex_lock(&lock);
    glUniform2f(surface_location, width, height);
    glViewport(0, 0, width, height);

    // set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // bind our vertex array
    glBindVertexArray(vertex_array);

    int max_lines = height / font_height;
    // vec4 vertex
    static std::vector<GLfloat> vertex_data;
    // vec3 textColor
    static std::vector<GLfloat> text_color_data;
    // vec3 backgroundColor
    static std::vector<GLfloat> background_color_data;

    vertex_data.clear();
    vertex_data.reserve(row * col * 24);
    text_color_data.clear();
    text_color_data.reserve(row * col * 18);
    background_color_data.clear();
    background_color_data.reserve(row * col * 18);

    // ensure at least one line shown, for very large scroll_offset
    int scroll_rows = scroll_offset / font_height;
    if ((int)history.size() + max_lines - 1 - scroll_rows < 0) {
        scroll_offset = ((int)history.size() + max_lines - 1) * font_height;
        scroll_rows = scroll_offset / font_height;
    }

    for (int i = 0; i < max_lines; i++) {
        // (height - font_height) is terminal[0] when scroll_offset is zero
        float x = 0.0;
        float y = height - (i + 1) * font_height;
        int i_row = i - scroll_rows;
        std::vector<term_char> ch;
        if (i_row >= 0 && i_row < term_row) {
            ch = terminal[i_row];
        } else if (i_row < 0 && (int)history.size() + i_row >= 0) {
            ch = history[history.size() + i_row];
        } else {
            continue;
        }

        int cur_col = 0;
        for (auto c : ch) {
            character ch = characters[c.ch][c.style.weight];
            float xpos = x;
            float ypos = y;
            float w = font_width;
            float h = font_height;

            // 1-2
            // | |
            // 3-4
            // (xpos    , ypos + h): 1
            // (xpos + w, ypos + h): 2
            // (xpos    , ypos    ): 3
            // (xpos + w, ypos    ): 4
            GLfloat g_vertex_buffer_data[24] = {// first triangle: 1->3->4
                                                xpos, ypos + h, ch.left, ch.top, xpos, ypos, ch.left, ch.bottom,
                                                xpos + w, ypos, ch.right, ch.bottom,
                                                // second triangle: 1->4->2
                                                xpos, ypos + h, ch.left, ch.top, xpos + w, ypos, ch.right, ch.bottom,
                                                xpos + w, ypos + h, ch.right, ch.top};
            vertex_data.insert(vertex_data.end(), &g_vertex_buffer_data[0], &g_vertex_buffer_data[24]);

            GLfloat g_text_color_buffer_data[18];
            GLfloat g_background_color_buffer_data[18];

            if (i_row == row && cur_col == col && show_cursor) {
                // cursor
                for (int i = 0; i < 6; i++) {
                    g_text_color_buffer_data[i * 3 + 0] = 1.0 - c.style.red;
                    g_text_color_buffer_data[i * 3 + 1] = 1.0 - c.style.green;
                    g_text_color_buffer_data[i * 3 + 2] = 1.0 - c.style.blue;
                    g_background_color_buffer_data[i * 3 + 0] = 0.0;
                    g_background_color_buffer_data[i * 3 + 1] = 0.0;
                    g_background_color_buffer_data[i * 3 + 2] = 0.0;
                }
            } else {
                for (int i = 0; i < 6; i++) {
                    g_text_color_buffer_data[i * 3 + 0] = c.style.red;
                    g_text_color_buffer_data[i * 3 + 1] = c.style.green;
                    g_text_color_buffer_data[i * 3 + 2] = c.style.blue;
                    g_background_color_buffer_data[i * 3 + 0] = 1.0;
                    g_background_color_buffer_data[i * 3 + 1] = 1.0;
                    g_background_color_buffer_data[i * 3 + 2] = 1.0;
                }
            }
            text_color_data.insert(text_color_data.end(), &g_text_color_buffer_data[0], &g_text_color_buffer_data[18]);
            background_color_data.insert(background_color_data.end(), &g_background_color_buffer_data[0],
                                         &g_background_color_buffer_data[18]);

            x += font_width;
            cur_col++;
        }
    }
    pthread_mutex_unlock(&lock);

    // draw in one pass
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertex_data.size(), vertex_data.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, text_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * text_color_data.size(), text_color_data.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, background_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * background_color_data.size(), background_color_data.data(),
                 GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, vertex_data.size() / 4);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFlush();
    glFinish();
    eglSwapBuffers(egl_display, egl_surface);
}

static void DropFirstRowIfOverflow() {
    if (row == term_row) {
        // drop first row
        history.push_back(terminal[0]);
        terminal.erase(terminal.begin());
        terminal.resize(term_row);
        terminal[term_row - 1].resize(term_col);
        row--;
        while (history.size() > MAX_HISTORY_LINES) {
            history.pop_front();
        }
    }
}

#define clamp_row()                                                                                                    \
    do {                                                                                                               \
        if (row < 0) {                                                                                                 \
            row = 0;                                                                                                   \
        } else if (row > term_row - 1) {                                                                               \
            row = term_row - 1;                                                                                        \
        }                                                                                                              \
    } while (0);

#define clamp_col()                                                                                                    \
    do {                                                                                                               \
        if (col < 0) {                                                                                                 \
            col = 0;                                                                                                   \
        } else if (col > term_col - 1) {                                                                               \
            col = term_col - 1;                                                                                        \
        }                                                                                                              \
    } while (0);

// CAUTION: clobbers temp
#define read_int_or_default(def)                                                                                       \
    (temp = 0, (escape_buffer != "" ? sscanf(escape_buffer.c_str(), "%d", &temp) : temp = (def)), temp)

static void *RenderWorker(void *) {
    pthread_setname_np(pthread_self(), "render worker");

    eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

    // build vertex and fragment shader
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    char const *vertex_source = "#version 320 es\n"
                                "\n"
                                "in vec4 vertex;\n"
                                "in vec3 textColor;\n"
                                "in vec3 backgroundColor;\n"
                                "out vec2 texCoords;\n"
                                "out vec3 outTextColor;\n"
                                "out vec3 outBackgroundColor;\n"
                                "uniform vec2 surface;\n"
                                "void main() {\n"
                                "  gl_Position.x = vertex.x / surface.x * 2.0f - 1.0f;\n"
                                "  gl_Position.y = vertex.y / surface.y * 2.0f - 1.0f;\n"
                                "  gl_Position.z = 0.0;\n"
                                "  gl_Position.w = 1.0;\n"
                                "  texCoords = vertex.zw;\n"
                                "  outTextColor = textColor;\n"
                                "  outBackgroundColor = backgroundColor;\n"
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
                                  "in vec3 outTextColor;\n"
                                  "in vec3 outBackgroundColor;\n"
                                  "out vec4 color;\n"
                                  "uniform sampler2D text;\n"
                                  "void main() {\n"
                                  "  float alpha = texture(text, texCoords).r;\n"
                                  "  color = vec4(outTextColor * alpha + outBackgroundColor * (1.0 - alpha), 1.0);\n"
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

    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        std::vector<char> link_program_error_message(info_log_length + 1);
        glGetProgramInfoLog(program_id, info_log_length, NULL, &link_program_error_message[0]);
        OH_LOG_INFO(LOG_APP, "Failed to link program: %{public}s", &link_program_error_message[0]);
    }

    surface_location = glGetUniformLocation(program_id, "surface");
    assert(surface_location != -1);

    glUseProgram(program_id);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // load font from ttf
    LoadFont();

    // create buffers for drawing
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    // vec4 vertex
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    GLint vertex_location = glGetAttribLocation(program_id, "vertex");
    assert(vertex_location != -1);
    glEnableVertexAttribArray(vertex_location);
    glVertexAttribPointer(vertex_location,   // attribute 0
                          4,                 // size
                          GL_FLOAT,          // type
                          GL_FALSE,          // normalized?
                          4 * sizeof(float), // stride
                          (void *)0          // array buffer offset
    );

    // vec3 textColor
    glGenBuffers(1, &text_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, text_color_buffer);
    GLint text_color_location = glGetAttribLocation(program_id, "textColor");
    assert(text_color_location != -1);
    glEnableVertexAttribArray(text_color_location);
    glVertexAttribPointer(text_color_location, // attribute 0
                          3,                   // size
                          GL_FLOAT,            // type
                          GL_FALSE,            // normalized?
                          3 * sizeof(float),   // stride
                          (void *)0            // array buffer offset
    );

    // vec3 backgroundColor
    glGenBuffers(1, &background_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, background_color_buffer);
    GLint background_color_location = glGetAttribLocation(program_id, "backgroundColor");
    assert(background_color_location != -1);
    glEnableVertexAttribArray(background_color_location);
    glVertexAttribPointer(background_color_location, // attribute 0
                          3,                         // size
                          GL_FLOAT,                  // type
                          GL_FALSE,                  // normalized?
                          3 * sizeof(float),         // stride
                          (void *)0                  // array buffer offset
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    struct timeval tv;
    gettimeofday(&tv, nullptr);
    uint64_t last_redraw_msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    uint64_t last_fps_msec = last_redraw_msec;
    Draw();
    int fps = 0;
    std::vector<uint64_t> time;
    while (1) {
        gettimeofday(&tv, nullptr);
        uint64_t now_msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;

        // even if we call faster than system settings (60Hz/120Hz), it does not get faster
        // 120 Hz, 8ms
        uint64_t deadline = last_redraw_msec + 8;
        if (now_msec < deadline) {
            usleep((deadline - now_msec) * 1000);
        }

        // redraw
        gettimeofday(&tv, nullptr);
        now_msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        last_redraw_msec = now_msec;
        Draw();

        gettimeofday(&tv, nullptr);
        uint64_t msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        time.push_back(msec - now_msec);

        fps++;

        // report fps
        if (now_msec - last_fps_msec > 1000) {
            last_fps_msec = now_msec;
            uint64_t sum = 0;
            for (auto t : time) {
                sum += t;
            }
            OH_LOG_INFO(LOG_APP, "FPS: %{public}d, %{public}ld ms per draw", fps, sum / fps);
            fps = 0;
            time.clear();
        }
    }
}

static void *TerminalWorker(void *) {
    pthread_setname_np(pthread_self(), "terminal worker");

    int temp = 0;
    // poll from fd, and render
    struct timeval tv;
    while (1) {
        struct pollfd fds[1];
        fds[0].fd = fd;
        fds[0].events = POLLIN;
        int res = poll(fds, 1, 1000);

        uint8_t buffer[1024];
        if (res > 0) {
            ssize_t r = read(fd, buffer, sizeof(buffer) - 1);
            if (r > 0) {
                // pretty print
                std::string hex;
                for (int i = 0; i < r; i++) {
                    if (buffer[i] >= 127 || buffer[i] < 32) {
                        char temp[8];
                        snprintf(temp, sizeof(temp), "\\x%02x", buffer[i]);
                        hex += temp;
                    } else {
                        hex += (char)buffer[i];
                    }
                }
                OH_LOG_INFO(LOG_APP, "Got: %{public}s", hex.c_str());

                // parse output
                pthread_mutex_lock(&lock);
                for (int i = 0; i < r; i++) {
                    if (escape_state == state_esc) {
                        if (buffer[i] == '[') {
                            // ESC [ = CSI
                            escape_state = state_csi;
                        } else if (buffer[i] == ']') {
                            // ESC ] = OSC
                            escape_state = state_osc;
                        }
                    } else if (escape_state == state_csi) {
                        if (buffer[i] == 'm') {
                            // set color
                            std::vector<std::string> parts = splitString(escape_buffer, ";");
                            for (auto part : parts) {
                                if (part == "1") {
                                    // set bold
                                    current_style.weight = weight::bold;
                                } else if (part == "31") {
                                    // red foreground
                                    current_style.red = 1.0;
                                    current_style.green = 0.0;
                                    current_style.blue = 0.0;
                                } else if (part == "32") {
                                    // green foreground
                                    current_style.red = 0.0;
                                    current_style.green = 1.0;
                                    current_style.blue = 0.0;
                                } else if (part == "34") {
                                    // blue foreground
                                    current_style.red = 0.0;
                                    current_style.green = 0.0;
                                    current_style.blue = 1.0;
                                } else if (part == "36") {
                                    // cyan foreground
                                    current_style.red = 0.0;
                                    current_style.green = 1.0;
                                    current_style.blue = 1.0;
                                } else if (part == "0") {
                                    // reset
                                    current_style = style();
                                }
                            }
                            if (escape_buffer == "") {
                                // reset
                                current_style = style();
                            }
                            escape_state = state_idle;
                        } else if (buffer[i] == 'A') {
                            // move cursor up # lines
                            row -= read_int_or_default(1);
                            clamp_row();
                            escape_state = state_idle;
                        } else if (buffer[i] == 'B') {
                            // move cursor down # lines
                            row += read_int_or_default(1);
                            clamp_row();
                            escape_state = state_idle;
                        } else if (buffer[i] == 'C') {
                            // move cursor right # columns
                            col += read_int_or_default(1);
                            clamp_col();
                            escape_state = state_idle;
                        } else if (buffer[i] == 'D') {
                            // move cursor left # columns
                            col -= read_int_or_default(1);
                            clamp_col();
                            escape_state = state_idle;
                        } else if (buffer[i] == 'd' && escape_buffer != "") {
                            // move cursor to row #
                            sscanf(escape_buffer.c_str(), "%d", &row);
                            // convert from 1-based to 0-based
                            row--;
                            clamp_row();
                            escape_state = state_idle;
                        } else if (buffer[i] == 'E') {
                            // move cursor to the beginning of next line, down # lines
                            row += read_int_or_default(1);
                            clamp_row();
                            col = 0;
                            escape_state = state_idle;
                        } else if (buffer[i] == 'F') {
                            // move cursor to the beginning of previous line, up # lines
                            row -= read_int_or_default(1);
                            clamp_row();
                            col = 0;
                            escape_state = state_idle;
                        } else if (buffer[i] == 'G') {
                            // move cursor to column #
                            col = read_int_or_default(1);
                            // convert from 1-based to 0-based
                            col--;
                            clamp_col();
                            escape_state = state_idle;
                        } else if (buffer[i] == 'h' && escape_buffer == "?25") {
                            // make cursor visible
                            show_cursor = true;
                            escape_state = state_idle;
                        } else if (buffer[i] == 'H' && escape_buffer == "") {
                            // move cursor to upper left corner
                            row = col = 0;
                            escape_state = state_idle;
                        } else if (buffer[i] == 'H' && escape_buffer != "") {
                            // move cursor to x, y
                            std::vector<std::string> parts = splitString(escape_buffer, ";");
                            if (parts.size() == 2) {
                                sscanf(parts[0].c_str(), "%d", &row);
                                sscanf(parts[1].c_str(), "%d", &col);
                                // convert from 1-based to 0-based
                                row--;
                                col--;
                                clamp_row();
                                clamp_col();
                            }
                            escape_state = state_idle;
                        } else if (buffer[i] == 'J' && (escape_buffer == "" || escape_buffer == "0")) {
                            // erase from cursor until end of screen
                            for (int i = col; i < term_col; i++) {
                                terminal[row][i] = term_char();
                            }
                            for (int i = row + 1; i < term_row; i++) {
                                std::fill(terminal[i].begin(), terminal[i].end(), term_char());
                            }
                            escape_state = state_idle;
                        } else if (buffer[i] == 'K' && (escape_buffer == "" || escape_buffer == "0")) {
                            // erase from cursor to end of line
                            for (int i = col; i < term_col; i++) {
                                terminal[row][i] = term_char();
                            }
                            escape_state = state_idle;
                        } else if (buffer[i] == 'K' && escape_buffer == "1") {
                            // erase from start of line to the cursor
                            for (int i = 0; i <= col; i++) {
                                if (i + col + 1 < term_col) {
                                    terminal[row][i] = terminal[row][i + col + 1];
                                } else {
                                    terminal[row][i] = term_char();
                                }
                            }
                            escape_state = state_idle;
                        } else if (buffer[i] == 'l' && escape_buffer == "?25") {
                            // make cursor invisible
                            show_cursor = false;
                            escape_state = state_idle;
                        } else if (buffer[i] == 'P' && escape_buffer != "") {
                            // erase # characters
                            int temp = 0;
                            sscanf(escape_buffer.c_str(), "%d", &temp);
                            for (int i = col; i < term_col; i++) {
                                if (i + temp < term_col) {
                                    terminal[row][i] = terminal[row][i + temp];
                                } else {
                                    terminal[row][i] = term_char();
                                }
                            }
                            escape_state = state_idle;
                        } else if (buffer[i] == '?' || buffer[i] == ';' || (buffer[i] >= '0' && buffer[i] <= '9')) {
                            // '?', ';' or number
                            escape_buffer += buffer[i];
                        } else {
                            // unknown
                            OH_LOG_INFO(LOG_APP, "Unknown escape sequence: %{public}s %{public}c",
                                        escape_buffer.c_str(), buffer[i]);
                            escape_state = state_idle;
                        }
                    } else if (escape_state == state_osc) {
                        if (buffer[i] == '\x07') {
                            // BEL, do nothing
                            escape_state = state_idle;
                        } else if (buffer[i] != '\x00') {
                            // not string terminator
                            escape_buffer += buffer[i];
                        } else {
                            // unknown
                            OH_LOG_INFO(LOG_APP, "Unknown escape sequence: %{public}s %{public}c",
                                        escape_buffer.c_str(), buffer[i]);
                            escape_state = state_idle;
                        }
                    } else {
                        // escape state is idle
                        if (buffer[i] >= ' ' && buffer[i] < 127) {
                            // printable
                            assert(row >= 0 && row < term_row);
                            assert(col >= 0 && col < term_col);
                            terminal[row][col].ch = buffer[i];
                            terminal[row][col].style = current_style;
                            col++;
                            if (col == term_col) {
                                col = 0;
                                row++;
                                DropFirstRowIfOverflow();
                            }
                        } else if (buffer[i] == '\r') {
                            col = 0;
                        } else if (buffer[i] == '\n') {
                            row += 1;
                            DropFirstRowIfOverflow();
                        } else if (buffer[i] == '\b') {
                            if (col > 0) {
                                col -= 1;
                            }
                        } else if (buffer[i] == '\t') {
                            col = (col + 8) / 8 * 8;
                            if (col >= term_col) {
                                col = 0;
                                row++;
                                DropFirstRowIfOverflow();
                            }
                        } else if (buffer[i] == 0x1b) {
                            escape_buffer = "";
                            escape_state = state_esc;
                        }
                    }
                }
                pthread_mutex_unlock(&lock);
            }
        }
    }
}

static napi_value CreateSurface(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    int64_t surface_id = 0;
    bool lossless = true;
    napi_status res = napi_get_value_bigint_int64(env, args[0], &surface_id, &lossless);
    assert(res == napi_ok);

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
    EGLBoolean egl_res = eglInitialize(egl_display, &major_version, &minor_version);
    assert(egl_res == EGL_TRUE);

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
    egl_res = eglChooseConfig(egl_display, attrib, &egl_config, max_config_size, &num_configs);
    assert(egl_res == EGL_TRUE);

    egl_surface = eglCreateWindowSurface(egl_display, egl_config, egl_window, NULL);

    EGLint context_attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, context_attributes);

    pthread_t render_thread;
    pthread_create(&render_thread, NULL, RenderWorker, NULL);

    pthread_t terminal_thread;
    pthread_create(&terminal_thread, NULL, TerminalWorker, NULL);
    return nullptr;
}

static napi_value ResizeSurface(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    napi_get_value_int32(env, args[1], &width);
    napi_get_value_int32(env, args[2], &height);

    pthread_mutex_lock(&lock);
    term_col = width / font_width;
    term_row = height / font_height;
    terminal.resize(term_row);
    for (int i = 0; i < term_row; i++) {
        terminal[i].resize(term_col);
    }

    if (row > term_row - 1) {
        row = term_row - 1;
    }

    if (col > term_col - 1) {
        col = term_col - 1;
    }
    pthread_mutex_unlock(&lock);

    struct winsize ws = {};
    ws.ws_col = term_col;
    ws.ws_row = term_row;
    ioctl(fd, TIOCSWINSZ, &ws);

    return nullptr;
}

static napi_value Scroll(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    double offset = 0;
    napi_status res = napi_get_value_double(env, args[0], &offset);
    assert(res == napi_ok);

    // natural scrolling
    scroll_offset -= offset;
    if (scroll_offset < 0) {
        scroll_offset = 0.0;
    }

    return nullptr;
}

static napi_value DestroySurface(napi_env env, napi_callback_info info) { return nullptr; }

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        {"run", nullptr, Run, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"send", nullptr, Send, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"createSurface", nullptr, CreateSurface, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"destroySurface", nullptr, DestroySurface, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"resizeSurface", nullptr, ResizeSurface, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"scroll", nullptr, Scroll, nullptr, nullptr, nullptr, napi_default, nullptr},
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
