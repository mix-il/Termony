// for standalone build to test on Linux:
// clang++ -std=c++17 terminal.cpp -I/usr/include/freetype2 -DSTANDALONE -lfreetype -lutf8proc -lGLESv2 -lglfw -o terminal

#include "terminal.h"
#include "freetype/ftmm.h"
#include "utf8proc-2.10.0/utf8proc.h"
#include <GLES3/gl32.h>
#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <poll.h>
#include <pty.h>
#include <pthread.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef STANDALONE
#define LOG_INFO(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)
#define LOG_WARN(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)
#define LOG_ERROR(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)
#else
#include "hilog/log.h"
#undef LOG_DEBUG
#undef LOG_INFO
#undef LOG_WARN
#undef LOG_ERROR
#undef LOG_FATAL
void hiprintf(int level, const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    constexpr int bufsz = 8192;
    char buf[bufsz];
    if (vsnprintf(buf, bufsz, fmt, args) > 0) {
        OH_LOG_Print(LOG_APP, (LogLevel)level, 0, "testTag", "%{public}s", buf);
    }
    va_end(args);
}
// supress logs
//#define hiprintf(...)
#define LOG_DEBUG(...) hiprintf(3, __VA_ARGS__)
#define LOG_INFO(...) hiprintf(4, __VA_ARGS__)
#define LOG_WARN(...) hiprintf(5, __VA_ARGS__)
#define LOG_ERROR(...) hiprintf(6, __VA_ARGS__)
#define LOG_FATAL(...) hiprintf(7, __VA_ARGS__)
#endif

// docs for escape codes:
// https://invisible-island.net/xterm/ctlseqs/ctlseqs.html
// https://vt100.net/docs/vt220-rm/chapter4.html
// https://espterm.github.io/docs/VT100%20escape%20codes.html
// https://ecma-international.org/wp-content/uploads/ECMA-48_5th_edition_june_1991.pdf
// https://xtermjs.org/docs/api/vtfeatures/
// terminology:
// C: a single character
// Ps: a single and optional numeric parameter
// Pm: list of Ps, separated by ;
// Pt: text parameter of printable characters

// solarized light
static uint32_t predefined_colors[max_term_color] = {
    [black] = PACK_RGB( 7, 54, 66 ),
    [red] = PACK_RGB( 220, 50, 47 ),
    [green] = PACK_RGB( 13, 153, 0 ),
    [yellow] = PACK_RGB( 181, 137, 0 ),
    [blue] = PACK_RGB( 38, 139, 210 ),
    [magenta] = PACK_RGB( 221, 54, 130 ),
    [cyan] = PACK_RGB( 42, 161, 152 ),
    [white] = PACK_RGB( 238, 232, 213 ),

    [brblack] = PACK_RGB( 0, 43, 54 ),
    [brred] = PACK_RGB( 203, 75, 22 ),
    [brgreen] = PACK_RGB( 88, 110, 117 ),
    [bryellow] = PACK_RGB( 101, 123, 131 ),
    [brblue] = PACK_RGB( 131, 148, 150 ),
    [brmagenta] = PACK_RGB( 108, 113, 196 ),
    [brcyan] = PACK_RGB( 147, 161, 161 ),
    [brwhite] = PACK_RGB( 253, 246, 227 ),
};

term_style::term_style() {
    fore = predefined_colors[black];
    back = predefined_colors[white];
}

static std::vector<std::string> SplitString(const std::string &str, const std::string &delimiter) {
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

// viewport width/height
static int vw100 = 0;
static int vh100 = 0;
static GLint surface_location = -1;
static GLint render_pass_location = -1;
#ifdef STANDALONE
// and this is scale = 1.0, too small on a HiDPI display
static int font_height = 24;
static int font_width = 12;
static int max_font_width = 24;
static int baseline_height = 5;
#else
// ohos screen is scaled but not 200% i think
static int font_height = 48;
static int font_width = 24;
static int max_font_width = 48;
static int baseline_height = 10;
#endif

// scroll offset in y axis
static float scroll_offset = 0;

const int MAX_HISTORY_LINES = 5000;

constexpr uint32_t TrueColorFrom(uint8_t index) {
    return color_map_256[index];
}

void terminal_context::ResizeTo(int new_term_row, int new_term_col) {
    int old_term_col = num_cols;
    num_rows = new_term_row;
    num_cols = new_term_col;

    // update scroll margin
    scroll_top = 0;
    scroll_bottom = num_rows - 1;

    buffer.resize(num_rows);
    for (int i = 0; i < num_rows; i++) {
        buffer[i].resize(num_cols);
    }

    if (row > num_rows - 1) {
        row = num_rows - 1;
    }

    if (col > num_cols - 1) {
        col = num_cols - 1;
    }

    tab_stops.resize(num_cols);
    for (int i = old_term_col;i < num_cols;i += tab_size) {
        tab_stops[i] = true;
    }

    struct winsize ws = {};
    ws.ws_col = num_cols;
    ws.ws_row = num_rows;
    ioctl(fd, TIOCSWINSZ, &ws);
}

void terminal_context::DropFirstRowIfOverflow() {
    if (row == scroll_bottom + 1) {
        // drop first row in scrolling margin
        assert(scroll_top < scroll_bottom);
        history.push_back(buffer[scroll_top]);
        buffer.erase(buffer.begin() + scroll_top);
        buffer.insert(buffer.begin() + scroll_bottom, std::vector<term_char>());
        buffer[scroll_bottom].resize(num_cols);
        row--;

        while (history.size() > MAX_HISTORY_LINES) {
            history.pop_front();
        }
    } else if (row >= num_rows) {
        row = num_rows - 1;
    }
}

int char_width(uint32_t codepoint) {
    return utf8proc_charwidth(codepoint);
}

void terminal_context::InsertUtf8(uint32_t codepoint) {
    assert(row >= 0 && row < num_rows);
    assert(col >= 0 && col <= num_cols);

    int cw = char_width(codepoint);
    // don't insert zero-width characters
    if (cw <= 0) return;
    // can fit if just equal num_cols
    if (col + cw > num_cols) {
        if (enable_wrap) {
            // wrap to next line
            row ++;
            col = 0;
            DropFirstRowIfOverflow();
        } else {
            // remove tail chars until fit
            col = num_cols - cw;
            // remove a broken wide char
            while (buffer[row][col].code == term_char::WIDE_TAIL)
                col --;
        }
    }
    if (cw > 1) {
        // place the wide char
        buffer[row][col].code = codepoint;
        buffer[row][col++].style = current_style;
        // and cw-2 spacers
        for (int i=1; i < cw-1 && col < num_cols; i++) {
            buffer[row][col].code = term_char::WIDE_TAIL;
            buffer[row][col++].style = current_style;
        }
        // final spacer can't be inserted
        if (col == num_cols) return;
        codepoint = term_char::WIDE_TAIL;
    }
    LOG_INFO("column: %d (%d)", col, cw);
    buffer[row][col].code = codepoint;
    buffer[row][col++].style = current_style;
}

// clamp cursor to valid range
void terminal_context::ClampCursor() {
    // clamp col
    if (col < 0) {
        col = 0;
    } else if (col > num_cols - 1) {
        col = num_cols - 1;
    }

    // clamp row
    if (origin_mode) {
        // limit cursor to scroll region
        if (row < scroll_top) {
            row = scroll_top;
        } else if (row > scroll_bottom) {
            row = scroll_bottom;
        }
    } else {
        // limit cursor to terminal
        if (row < 0) {
            row = 0;
        } else if (row > num_rows - 1) {
            row = num_rows - 1;
        }
    }
}

// set absolute cursor location
void terminal_context::SetCursor(int new_row, int new_col) {
    if (origin_mode) {
        // origin mode, home position is the scrolling top
        row = new_row + scroll_top;
        col = new_col;
    } else {
        row = new_row;
        col = new_col;
    }
    ClampCursor();
}

// move cursor in relative position
void terminal_context::MoveCursor(int row_diff, int col_diff) {
    ClampCursor();
    SetCursor(row + row_diff, col + col_diff);
}

// write data to pty until fully sent
void terminal_context::WriteFull(uint8_t *data, size_t length) {
    if (fd == -1) {
        return;
    }

    // pretty print
    std::string hex;
    for (int i = 0; i < length; i++) {
        if (data[i] >= 127 || data[i] < 32) {
            char temp[8];
            snprintf(temp, sizeof(temp), "\\x%02x", data[i]);
            hex += temp;
        } else {
            hex += (char)data[i];
        }
    }
    LOG_INFO("Send: %s", hex.c_str());

    int written = 0;
    while (written < length) {
        int size = write(fd, (uint8_t *)data + written, length - written);
        assert(size >= 0);
        written += size;
    }
}

// CAUTION: clobbers temp
#define read_int_or_default(def)                                                                                       \
    (temp = 0, (escape_buffer != "" ? sscanf(escape_buffer.c_str(), "%d", &temp) : temp = (def)), temp)

// handle CSI escape sequences
void terminal_context::HandleCSI(uint8_t current) {
    int temp = 0;
    if (current >= 0x40 && current <= 0x7E) {
        // final byte in [0x40, 0x7E]
        if (current == 'A') {
            // CSI Ps A, CUU, move cursor up # lines
            int line = read_int_or_default(1);
            if (row >= scroll_top) {
                // do not move past scrolling margin
                MoveCursor(-std::min(line, row - scroll_top), 0);
            } else {
                // we are out of scrolling region, move nevertheless
                MoveCursor(-line, 0);
            }
        } else if (current == 'B') {
            // CSI Ps B, CUD, move cursor down # lines
            int line = read_int_or_default(1);
            if (row <= scroll_bottom) {
                // do not move past scrolling margin
                MoveCursor(std::min(line, scroll_bottom - row), 0);
            } else {
                // we are out of scrolling region, move nevertheless
                MoveCursor(line,  0);
            }
        } else if (current == 'C') {
            // CSI Ps C, CUF, move cursor right # columns
            col += std::max(read_int_or_default(1), 1);
            ClampCursor();
        } else if (current == 'D') {
            // CSI Ps D, CUB, move cursor left # columns
            col -= std::max(read_int_or_default(1), 1);
            ClampCursor();
        } else if (current == 'E') {
            // CSI Ps E, CNL, move cursor to the beginning of next line, down # lines
            row += read_int_or_default(1);
            col = 0;
            ClampCursor();
        } else if (current == 'F') {
            // CSI Ps F, CPL, move cursor to the beginning of previous line, up # lines
            row -= read_int_or_default(1);
            col = 0;
            ClampCursor();
        } else if (current == 'G') {
            // CSI Ps G, CHA, move cursor to column #
            col = read_int_or_default(1);
            // convert from 1-based to 0-based
            col--;
            ClampCursor();
        } else if (current == 'H') {
            std::vector<std::string> parts = SplitString(escape_buffer, ";");
            if (parts.size() == 2) {
                // CSI Ps ; PS H, CUP, move cursor to x, y
                sscanf(parts[0].c_str(), "%d", &row);
                sscanf(parts[1].c_str(), "%d", &col);
                // convert from 1-based to 0-based
                row--;
                col--;
                ClampCursor();
            } else if (parts.size() == 1 && escape_buffer != "") {
                // CSI Ps H, CUP, move cursor to x, y, col is 0
                sscanf(parts[0].c_str(), "%d", &row);
                col = 0;
                // convert from 1-based to 0-based
                row--;
                ClampCursor();
            } else if (escape_buffer == "") {
                // CSI H, HOME, move cursor upper left corner
                row = col = 0;
            } else {
                goto unknown;
            }
        } else if (current == 'J') {
            // CSI Ps J, ED, erase in display
            if (escape_buffer == "" || escape_buffer == "0") {
                // CSI J, CSI 0 J
                // erase below
                for (int i = col; i < num_cols; i++) {
                    buffer[row][i] = term_char();
                }
                for (int i = row + 1; i < num_rows; i++) {
                    std::fill(buffer[i].begin(), buffer[i].end(), term_char());
                }
            } else if (escape_buffer == "1") {
                // CSI 1 J
                // erase above
                for (int i = 0; i < row; i++) {
                    std::fill(buffer[i].begin(), buffer[i].end(), term_char());
                }
                for (int i = 0; i <= col; i++) {
                    buffer[row][i] = term_char();
                }
            } else if (escape_buffer == "2") {
                // CSI 2 J
                // erase all
                for (int i = 0; i < num_rows; i++) {
                    std::fill(buffer[i].begin(), buffer[i].end(), term_char());
                }
            } else {
                goto unknown;
            }
        } else if (current == 'K') {
            // CSI Ps K, EL, erase in line
            if (escape_buffer == "" || escape_buffer == "0") {
                // CSI K, CSI 0 K
                // erase to right
                for (int i = col; i < num_cols; i++) {
                    buffer[row][i] = term_char();
                }
            } else if (escape_buffer == "1") {
                // CSI 1 K
                // erase to left
                for (int i = 0; i <= col && i < num_cols; i++) {
                    buffer[row][i] = term_char();
                }
            } else if (escape_buffer == "2") {
                // CSI 2 K
                // erase whole line
                for (int i = 0; i < num_cols; i++) {
                    buffer[row][i] = term_char();
                }
            } else {
                goto unknown;
            }
        } else if (current == 'L') {
            // CSI Ps L, Insert Ps blank lines at active row
            int line = read_int_or_default(1);
            if (row < scroll_top || row > scroll_bottom) {
                // outside the scroll margins, do nothing
            } else {
                // insert lines from current row, add new rows from scroll bottom
                for (int i = scroll_bottom;i >= row;i --) {
                    if (i - line >= row) {
                        buffer[i] = buffer[i - line];
                    } else {
                        std::fill(buffer[i].begin(), buffer[i].end(), term_char());
                    }
                }
                // set to first column
                col = 0;
            }
        } else if (current == 'M') {
            // CSI Ps M, Delete Ps lines at active row
            int line = read_int_or_default(1);
            if (row < scroll_top || row > scroll_bottom) {
                // outside the scroll margins, do nothing
            } else {
                // delete lines from current row, add new rows from scroll bottom
                for (int i = row;i <= scroll_bottom;i ++) {
                    if (i + line <= scroll_bottom) {
                        buffer[i] = buffer[i + line];
                    } else {
                        std::fill(buffer[i].begin(), buffer[i].end(), term_char());
                    }
                }
                // set to first column
                col = 0;
            }
        } else if (current == 'P') {
            // CSI Ps P, DCH, delete # characters, move right to left
            int del = read_int_or_default(1);
            for (int i = col; i < num_cols; i++) {
                if (i + del < num_cols) {
                    buffer[row][i] = buffer[row][i + del];
                } else {
                    buffer[row][i] = term_char();
                }
            }
        } else if (current == 'S') {
            // CSI Ps S, SU, Scroll up Ps lines
            int line = read_int_or_default(1);
            for (int i = scroll_top; i <= scroll_bottom; i++) {
                if (i + line <= scroll_bottom) {
                    buffer[i] = buffer[i + line];
                } else {
                    std::fill(buffer[i].begin(), buffer[i].end(), term_char());
                }
            }
        } else if (current == 'X') {
            // CSI Ps X, ECH, erase # characters, do not move others
            int del = read_int_or_default(1);
            for (int i = col; i < col + del && i < num_cols; i++) {
                buffer[row][i] = term_char();
            }
        } else if (current == 'c' && (escape_buffer == "" || escape_buffer == "0")) {
            // CSI Ps c, Send Device Attributes, Primary DA
            // mimic xterm
            // send CSI ? 1 ; 2 c: I am VT100 with Advance Video Option
            uint8_t send_buffer[] = {0x1b, '[', '?', '1', ';', '2', 'c'};
            WriteFull(send_buffer, sizeof(send_buffer));
        } else if (current == 'c' && (escape_buffer == ">" || escape_buffer == ">0")) {
            // CSI > Ps c, Send Device Attributes, Secondary DA
            // mimic xterm
            // send CSI > 0 ; 2 7 6 ; 0 c: I am VT100
            uint8_t send_buffer[] = {0x1b, '[', '>', '0', ';', '2', '7', '6', ';', '0', 'c'};
            WriteFull(send_buffer, sizeof(send_buffer));
        } else if (current == 'd' && escape_buffer != "") {
            // CSI Ps d, VPA, move cursor to row #
            sscanf(escape_buffer.c_str(), "%d", &row);
            // convert from 1-based to 0-based
            row--;
            ClampCursor();
        } else if (current == 'f') {
            std::vector<std::string> parts = SplitString(escape_buffer, ";");
            if (parts.size() == 2) {
                // CSI Ps ; PS f, CUP, move cursor to x, y
                sscanf(parts[0].c_str(), "%d", &row);
                sscanf(parts[1].c_str(), "%d", &col);
                // convert from 1-based to 0-based
                row--;
                col--;
                ClampCursor();
            } else {
                goto unknown;
            }
        } else if (current == 'g') {
            int mode = read_int_or_default(0);
            if (mode == 0) {
                // CSI g, CSI 0 g, clear tab stop at the current position
                tab_stops[col] = false;
            } else if (mode == 3) {
                // CSI 3 g, clear all tab stops
                std::fill(tab_stops.begin(), tab_stops.end(), false);
            } else {
                goto unknown;
            }
        } else if (current == 'h' && escape_buffer.size() > 0 && escape_buffer[0] != '?') {
            // CSI Pm h, Set Mode, SM
            std::vector<std::string> parts = SplitString(escape_buffer, ";");
            for (auto part : parts) {
                if (part == "4") {
                    // CSI 4 h, Insert Mode (IRM)
                    insert_mode = true;
                } else {
                    LOG_WARN("Unknown CSI Pm h: %s %c",
                                escape_buffer.c_str(), current);
                }
            }
        } else if (current == 'h' && escape_buffer.size() > 0 && escape_buffer[0] == '?') {
            // CSI ? Pm h, DEC Private Mode Set (DECSET)
            std::vector<std::string> parts = SplitString(escape_buffer.substr(1), ";");
            for (auto part : parts) {
                if (part == "1") {
                    // CSI ? 1 h, Application Cursor Keys (DECCKM)
                    // TODO
                } else if (part == "3") {
                    // CSI ? 3 h, Enable 132 Column mode, DECCOLM
                    ResizeTo(num_rows, 132);
                    ResizeWidth(132 * font_width);
                } else if (part == "4") {
                    // CSI ? 4 h, Smooth (Slow) Scroll (DECSCLM)
                    // TODO
                } else if (part == "5") {
                    // CSI ? 5 h, Reverse Video (DECSCNM)
                    reverse_video = true;
                } else if (part == "6") {
                    // CSI ? 6 h, Origin Mode (DECOM)
                    origin_mode = true;
                } else if (part == "7") {
                    // CSI ? 7 h, Set autowrap
                    enable_wrap = true;
                } else if (part == "12") {
                    // CSI ? 12 h, Start blinking cursor
                    // TODO
                } else if (part == "25") {
                    // CSI ? 25 h, DECTCEM, make cursor visible
                    show_cursor = true;
                } else if (part == "40") {
                    // CSI ? 40 h, Allow 80 -> 132 mode, xterm
                    // TODO
                } else if (part == "1000") {
                    // CSI ? 1000 h, Send Mouse X & Y on button press and release
                    // TODO
                } else if (part == "1002") {
                    // CSI ? 1002 h, Use Cell Motion Mouse Tracking
                    // TODO
                } else if (part == "1006") {
                    // CSI ? 1006 h, Enable SGR Mouse Mode
                    // TODO
                } else if (part == "2004") {
                    // CSI ? 2004 h, set bracketed paste mode
                    // TODO
                } else {
                    LOG_WARN("Unknown CSI ? Pm h: %s %c",
                                escape_buffer.c_str(), current);
                }
            }
        } else if (current == 'l' && escape_buffer.size() > 0 && escape_buffer[0] != '?') {
            // CSI Pm l, Reset Mode, RM
            std::vector<std::string> parts = SplitString(escape_buffer, ";");
            for (auto part : parts) {
                if (part == "4") {
                    // CSI 4 l, Replace Mode (IRM)
                    insert_mode = false;
                } else {
                    LOG_WARN("Unknown CSI Pm h: %s %c",
                                escape_buffer.c_str(), current);
                }
            }
        } else if (current == 'l' && escape_buffer.size() > 0 && escape_buffer[0] == '?') {
            // CSI ? Pm l, DEC Private Mode Reset (DECRST)
            std::vector<std::string> parts = SplitString(escape_buffer.substr(1), ";");
            for (auto part : parts) {
                if (part == "1") {
                    // CSI ? 1 l, Normal Cursor Keys (DECCKM)
                    // TODO
                } else if (part == "3") {
                    // CSI ? 3 l, 80 Column Mode (DECCOLM)
                    ResizeTo(num_rows, 80);
                    ResizeWidth(80 * font_width);
                } else if (part == "4") {
                    // CSI ? 4 l, Jump (Fast) Scroll (DECSCLM)
                    // TODO
                } else if (part == "5") {
                    // CSI ? 5 l, Normal Video (DECSCNM)
                    reverse_video = false;
                } else if (part == "6") {
                    // CSI ? 6 l, Normal Cursor Mode (DECOM)
                    origin_mode = false;
                } else if (part == "7") {
                    // CSI ? 7 l, Reset autowrap
                    enable_wrap = false;
                } else if (part == "8") {
                    // CSI ? 8 l, No Auto-Repeat Keys (DECARM)
                    // TODO
                } else if (part == "12") {
                    // CSI ? 12 l, Stop blinking cursor
                    // TODO
                } else if (part == "25") {
                    // CSI ? 25 l, Hide cursor (DECTCEM)
                    show_cursor = true;
                } else if (part == "45") {
                    // CSI ? 40 l, Disable Graphic Print Color Syntax (DECGPCS)
                    // TODO
                } else if (part == "2004") {
                    // CSI ? 2004 l, reset bracketed paste mode
                    // TODO
                } else {
                    LOG_WARN("Unknown CSI ? Pm l: %s %c",
                                escape_buffer.c_str(), current);
                }
            }
        } else if (current == 'm' && (escape_buffer.size() == 0 || escape_buffer[0] != '>')) {
            // CSI Pm m, Character Attributes (SGR)

            // set color
            std::vector<std::string> parts = SplitString(escape_buffer, ";");
            for (size_t i = 0; i < parts.size(); i++) {
                int param = 0;
                std::string part = parts[i];
                sscanf(part.c_str(), "%d", &param);
                if (param == 0) {
                    // reset all attributes to their defaults
                    current_style = term_style();
                } else if (param == 1) {
                    // set bold, CSI 1 m
                    current_style.weight = font_weight::bold;
                } else if (param == 2) {
                    // set faint, CSI 2 m
                    // TODO
                } else if (param == 4) {
                    // set underline, CSI 4 m
                    // TODO
                } else if (param == 5 || param == 6) {
                    // set slowly blink, CSI 5 m
                    // set rapidly blink, CSI 6 m
                    current_style.blink = true;
                } else if (param == 7) {
                    // inverse, flip foreground and background color, CSI 7 m
                    std::swap(current_style.fore, current_style.back);
                } else if (param == 9) {
                    // set strikethrough, CSI 9 m
                    // TODO
                } else if (param == 10) {
                    // reset to primary font, CSI 10 m
                    current_style = term_style();
                } else if (param == 21) {
                    // set doubly underlined, CSI 21 m
                    // TODO
                } else if (param == 22) {
                    // set not bold faint, CSI 22 m
                    current_style.weight = font_weight::regular;
                } else if (param == 24) {
                    // set not underlined, CSI 24 m
                    // TODO
                } else if (param == 25) {
                    // set steady (not blinking), CSI 25 m
                    current_style.blink = false;
                } else if (param == 27) {
                    // set positive (not inverse), CSI 27 m
                    std::swap(current_style.fore, current_style.back);
                } else if (30 <= param && param <= 37) {
                    // foreground ansi 0..7
                    current_style.fore = predefined_colors[param - 30];
                } else if (param == 38 || param == 48) {
                    // foreground color: extended color, CSI 38 : ... m
                    // background color: extended color, CSI 48 : ... m
                    if (i + 1 < parts.size()) {
                        int color_type = std::stoi(parts[++i]);
                        if (color_type == 5 && i + 1 < parts.size()) { // 256-color mode
                            // specified color index
                            int color_index = std::stoi(parts[++i]);
                            uint32_t color = TrueColorFrom(color_index);
                            if (param == 38) {
                                current_style.fore = color;
                            } else {
                                current_style.back = color;
                            }
                        } else if (color_type == 2 && i + 3 < parts.size()) { // RGB mode
                            // specified rgb
                            int r = std::stoi(parts[++i]);
                            int g = std::stoi(parts[++i]);
                            int b = std::stoi(parts[++i]);
                            if (param == 38) {
                                current_style.fore.set_rgb(r,g,b);
                            } else {
                                current_style.back.set_rgb(r,g,b);
                            }
                        }
                    }
                } else if (param == 39) {
                    // default foreground
                    current_style.fore = predefined_colors[black];
                } else if (40 <= param && param <= 47) {
                    // background ansi 0..7
                    current_style.back = predefined_colors[param - 40];
                } else if (param == 49) {
                    // default background
                    current_style.back = predefined_colors[white];
                } else if (90 <= param && param <= 97) {
                    // foreground ansi 8..15
                    current_style.fore = predefined_colors[8 + param - 90];
                } else if (100 <= param && param <= 107) {
                    // background ansi 8..15
                    current_style.back = predefined_colors[8 + param - 100];
                } else {
                    LOG_WARN("Unknown CSI Pm m: %s from %s %c",
                                part.c_str(), escape_buffer.c_str(), current);
                }
            }
        } else if (current == 'm' && escape_buffer.size() > 0 && escape_buffer[0] == '>') {
            // CSI > Pp m, XTMODKEYS, set/reset key modifier options
            // TODO
        } else if (current == 'n' && escape_buffer == "5") {
            // CSI 5 n - Device Status Report
            // send "OK" - ESC [ 0 n
            uint8_t ok_response[] = {0x1B, '[', '0', 'n'};
            WriteFull(ok_response, sizeof(ok_response));
        } else if (current == 'n' && (escape_buffer == "6")) {
            // CSI Ps n, DSR, Device Status Report
            // Ps = 6: Report Cursor Position (CPR)
            // send ESC [ row ; col R
            char send_buffer[128] = {};
            snprintf(send_buffer, sizeof(send_buffer), "\x1b[%d;%dR", row + 1, col + 1);
            int len = strlen(send_buffer);
            WriteFull((uint8_t *)send_buffer, len);
        } else if (current == 'r') {
            // CSI Ps ; Ps r, Set Scrolling Region [top;bottom]
            std::vector<std::string> parts = SplitString(escape_buffer, ";");
            int new_top = 1;
            int new_bottom = num_rows;
            if (parts.size() == 2) {
                // CSI Ps ; Ps r
                sscanf(parts[0].c_str(), "%d", &new_top);
                sscanf(parts[1].c_str(), "%d", &new_bottom);
                // convert to 1-based
                new_top --;
                new_bottom --;
            } else if (escape_buffer == "") {
                // full size of window
                // CSI r
                new_top = 0;
                new_bottom = num_rows - 1;
            } else if (parts.size() == 1) {
                // CSI Ps r
                sscanf(parts[0].c_str(), "%d", &new_top);
                // convert to 1-based
                new_top --;
                new_bottom = num_rows - 1;
            } else {
                goto unknown;
            }

            if (new_bottom > new_top) {
                scroll_top = new_top;
                scroll_bottom = new_bottom;

                // move cursor to new home position
                row = scroll_top;
                col = 0;
            }
        } else if (current == '@' &&
                ((escape_buffer.size() > 0 && escape_buffer[escape_buffer.size() - 1] >= '0' &&
                    escape_buffer[escape_buffer.size() - 1] <= '9') ||
                    escape_buffer == "")) {
            // CSI Ps @, ICH, Insert Ps (Blank) Character(s)
            int count = read_int_or_default(1);
            for (int i = num_cols - 1; i >= col; i--) {
                if (i - col < count) {
                    buffer[row][i].code = ' ';
                } else {
                    buffer[row][i] = buffer[row][i - count];
                }
            }
        } else {
unknown:
            // unknown
            LOG_WARN("Unknown escape sequence in CSI: %s %c",
                        escape_buffer.c_str(), current);
        }
        escape_state = state_idle;
    } else if (current >= 0x20 && current <= 0x3F) {
        // parameter bytes in [0x30, 0x3F],
        // or intermediate bytes in [0x20, 0x2F]
        escape_buffer += current;
    } else {
        // invalid byte
        // unknown
        LOG_WARN("Unknown escape sequence in CSI: %s %c",
                    escape_buffer.c_str(), current);
        escape_state = state_idle;
    }
}

void *terminal_context::TerminalWorker(void * data) {
    terminal_context *ctx = (terminal_context *)data;
    ctx->Worker();
    return NULL;
}

void terminal_context::Parse(uint8_t input) {
    if (escape_state == state_esc) {
        if (input == '[' && escape_buffer == "") {
            // ESC [ = CSI
            escape_state = state_csi;
        } else if (input == ']' && escape_buffer == "") {
            // ESC ] = OSC
            escape_state = state_osc;
        } else if (input == '=' && escape_buffer == "") {
            // ESC =, enter alternate keypad mode
            // TODO
            escape_state = state_idle;
        } else if (input == '>' && escape_buffer == "") {
            // ESC >, exit alternate keypad mode
            // TODO
            escape_state = state_idle;
        } else if (input == 'A' && escape_buffer == "") {
            // ESC A, cursor up
            row --;
            ClampCursor();
            escape_state = state_idle;
        } else if (input == 'B' && escape_buffer == "") {
            // ESC B, cursor down
            row ++;
            ClampCursor();
            escape_state = state_idle;
        } else if (input == 'C' && escape_buffer == "") {
            // ESC C, cursor right
            col ++;
            ClampCursor();
            escape_state = state_idle;
        } else if (input == 'D' && escape_buffer == "") {
            // ESC D, IND, cursor down and scroll
            row += 1;
            DropFirstRowIfOverflow();
            escape_state = state_idle;
        } else if (input == 'E' && escape_buffer == "") {
            // ESC E, goto to the beginning of next row
            row ++;
            col = 0;
            ClampCursor();
            escape_state = state_idle;
        } else if (input == 'H' && escape_buffer == "") {
            // ESC H, place tab stop at the current position
            tab_stops[col] = true;
            escape_state = state_idle;
        } else if (input == 'M' && escape_buffer == "") {
            // ESC M, move cursor one line up, scrolls down if at the top margin
            if (row == scroll_top) {
                // shift rows down
                for (int i = scroll_bottom;i > scroll_top;i--) {
                    buffer[i] = buffer[i-1];
                }
                std::fill(buffer[scroll_top].begin(), buffer[scroll_top].end(), term_char());
            } else {
                row --;
                ClampCursor();
            }
            escape_state = state_idle;
        } else if (input == 'P' && escape_buffer == "") {
            // ESC P = DCS
            escape_state = state_dcs;
        } else if (input == '8' && escape_buffer == "#") {
            // ESC # 8, DECALN fill viewport with a test pattern (E)
            for (int i = 0;i < num_rows;i++) {
                for (int j = 0;j < num_cols;j++) {
                    buffer[i][j] = term_char();
                    buffer[i][j].code = 'E';
                }
            }
            escape_state = state_idle;
        } else if (input == '7' && escape_buffer == "") {
            // ESC 7, save cursor
            save_row = row;
            save_col = col;
            save_style = current_style;
            escape_state = state_idle;
        } else if (input == '8' && escape_buffer == "") {
            // ESC 8, restore cursor
            row = save_row;
            col = save_col;
            ClampCursor();
            current_style = save_style;
            escape_state = state_idle;
        } else if (input == '#' || input == '(' || input == ')') {
            // non terminal
            escape_buffer += input;
        } else {
            // unknown
            LOG_WARN("Unknown escape sequence after ESC: %s %c",
                        escape_buffer.c_str(), input);
            escape_state = state_idle;
        }
    } else if (escape_state == state_csi) {
        HandleCSI(input);
    } else if (escape_state == state_osc) {
        if (input == '\x07') {
            // OSC Ps ; Pt BEL
            std::vector<std::string> parts = SplitString(escape_buffer, ";");
            if (parts.size() == 3 && parts[0] == "52" && parts[1] == "c" && parts[2] != "?") {
                // OSC 52 ; c ; BASE64 BEL
                // copy to clipboard
                std::string base64 = parts[2];
                LOG_INFO("Copy to pasteboard in native: %s",
                            base64.c_str());
                Copy(base64);
            } else if (parts.size() == 3 && parts[0] == "52" && parts[1] == "c" && parts[2] == "?") {
                // OSC 52 ; c ; ? BEL
                // paste from clipboard
                RequestPaste();
                LOG_INFO("Request Paste from pasteboard: %s", escape_buffer.c_str());
            }
            escape_state = state_idle;
        } else if (input == '\\' && escape_buffer.size() > 0 && escape_buffer[escape_buffer.size() - 1] == '\x1b') {
            // ST is ESC \
            // OSC Ps ; Pt ST
            std::vector<std::string> parts = SplitString(escape_buffer.substr(0, escape_buffer.size() - 1), ";");
            if (parts.size() == 2 && parts[0] == "10" && parts[1] == "?") {
                // OSC 10 ; ? ST
                // report foreground color: black
                // send OSI 10 ; r g b : 0 / 0 / 0 ST
                uint8_t send_buffer[] = {0x1b, ']', '1', '0', ';', 'r', 'g', 'b', ':', '0', '/', '0', '/', '0', '\x1b', '\\'};
                WriteFull(send_buffer, sizeof(send_buffer));
            } else if (parts.size() == 2 && parts[0] == "11" && parts[1] == "?") {
                // OSC 11 ; ? ST
                // report background color: white
                // send OSI 11 ; r g b : f / f / f ST
                uint8_t send_buffer[] = {0x1b, ']', '1', '0', ';', 'r', 'g', 'b', ':', 'f', '/', 'f', '/', 'f', '\x1b', '\\'};
                WriteFull(send_buffer, sizeof(send_buffer));
            }
            escape_state = state_idle;
        } else if ((input >= ' ' && input < 127) || input == '\x1b') {
            // printable character
            escape_buffer += input;
        } else {
            // unknown
            LOG_WARN("Unknown escape sequence in OSC: %s %c",
                        escape_buffer.c_str(), input);
            escape_state = state_idle;
        }
    } else if (escape_state == state_dcs) {
        if (input == '\\' && escape_buffer.size() > 0 && escape_buffer[escape_buffer.size() - 1] == '\x1b') {
            // ST
            escape_state = state_idle;
        } else if (input >= ' ' && input < 127 && input == '\x1b') {
            // printable character
            escape_buffer += input;
        } else {
            // unknown
            LOG_WARN("Unknown escape sequence in DCS: %s %c",
                        escape_buffer.c_str(), input);
            escape_state = state_idle;
        }
    } else if (escape_state == state_idle) {
        // escape state is idle
        if (utf8_state == state_initial) {
            if (input >= ' ' && input <= 0x7f) {
                // printable
                if (insert_mode) {
                    // move characters rightward
                    for (int i = num_cols - 1;i > col;i--) {
                        buffer[row][i] = buffer[row][i - 1];
                    }
                }
                InsertUtf8(input);
            } else if (input >= 0xc2 && input <= 0xdf) {
                // 2-byte utf8
                utf8_state = state_2byte_2;
                current_utf8 = (uint32_t)(input & 0x1f) << 6;
            } else if (input == 0xe0) {
                // 3-byte utf8 starting with e0
                utf8_state = state_3byte_2_e0;
                current_utf8 = (uint32_t)(input & 0x0f) << 12;
            } else if (input >= 0xe1 && input <= 0xef) {
                // 3-byte utf8 starting with non-e0
                utf8_state = state_3byte_2_non_e0;
                current_utf8 = (uint32_t)(input & 0x0f) << 12;
            } else if (input == 0xf0) {
                // 4-byte utf8 starting with f0
                utf8_state = state_4byte_2_f0;
                current_utf8 = (uint32_t)(input & 0x07) << 18;
            } else if (input >= 0xf1 && input <= 0xf3) {
                // 4-byte utf8 starting with f1 to f3
                utf8_state = state_4byte_2_f1_f3;
                current_utf8 = (uint32_t)(input & 0x07) << 18;
            } else if (input == 0xf4) {
                // 4-byte utf8 starting with f4
                utf8_state = state_4byte_2_f4;
                current_utf8 = (uint32_t)(input & 0x07) << 18;
            } else if (input == '\r') {
                col = 0;
            } else if (input == '\n') {
                // CUD1=\n, cursor down by 1
                row += 1;
                DropFirstRowIfOverflow();
            } else if (input == '\b') {
                // CUB1=^H, cursor backward by 1
                if (col > 0) {
                    col -= 1;
                }
            } else if (input == '\t') {
                // goto next tab stop
                col ++;
                while (col < num_cols && !tab_stops[col]) {
                    col ++;
                }
                ClampCursor();
            } else if (input == 0x1b) {
                escape_buffer = "";
                escape_state = state_esc;
            }
        } else if (utf8_state == state_2byte_2) {
            // expecting the second byte of 2-byte utf-8
            if (input >= 0x80 && input <= 0xbf) {
                current_utf8 |= (input & 0x3f);
                InsertUtf8(current_utf8);
            }
            utf8_state = state_initial;
        } else if (utf8_state == state_3byte_2_e0) {
            // expecting the second byte of 3-byte utf-8 starting with 0xe0
            if (input >= 0xa0 && input <= 0xbf) {
                current_utf8 |= (uint32_t)(input & 0x3f) << 6;
                utf8_state = state_3byte_3;
            } else {
                utf8_state = state_initial;
            }
        } else if (utf8_state == state_3byte_2_non_e0) {
            // expecting the second byte of 3-byte utf-8 starting with non-0xe0
            if (input >= 0x80 && input <= 0xbf) {
                current_utf8 |= (uint32_t)(input & 0x3f) << 6;
                utf8_state = state_3byte_3;
            } else {
                utf8_state = state_initial;
            }
        } else if (utf8_state == state_3byte_3) {
            // expecting the third byte of 3-byte utf-8 starting with 0xe0
            if (input >= 0x80 && input <= 0xbf) {
                current_utf8 |= (input & 0x3f);
                InsertUtf8(current_utf8);
            }
            utf8_state = state_initial;
        } else if (utf8_state == state_4byte_2_f0) {
            // expecting the second byte of 4-byte utf-8 starting with 0xf0
            if (input >= 0x90 && input <= 0xbf) {
                current_utf8 |= (uint32_t)(input & 0x3f) << 12;
                utf8_state = state_4byte_3;
            } else {
                utf8_state = state_initial;
            }
        } else if (utf8_state == state_4byte_2_f1_f3) {
            // expecting the second byte of 4-byte utf-8 starting with 0xf0 to 0xf3
            if (input >= 0x80 && input <= 0xbf) {
                current_utf8 |= (uint32_t)(input & 0x3f) << 12;
                utf8_state = state_4byte_3;
            } else {
                utf8_state = state_initial;
            }
        } else if (utf8_state == state_4byte_2_f4) {
            // expecting the second byte of 4-byte utf-8 starting with 0xf4
            if (input >= 0x80 && input <= 0x8f) {
                current_utf8 |= (uint32_t)(input & 0x3f) << 12;
                utf8_state = state_4byte_3;
            } else {
                utf8_state = state_initial;
            }
        } else if (utf8_state == state_4byte_3) {
            // expecting the third byte of 4-byte utf-8
            if (input >= 0x80 && input <= 0xbf) {
                current_utf8 |= (uint32_t)(input & 0x3f) << 6;
                utf8_state = state_4byte_4;
            } else {
                utf8_state = state_initial;
            }
        } else if (utf8_state == state_4byte_4) {
            // expecting the third byte of 4-byte utf-8
            if (input >= 0x80 && input <= 0xbf) {
                current_utf8 |= (input & 0x3f);
                InsertUtf8(current_utf8);
            }
            utf8_state = state_initial;
        } else {
            assert(false && "unreachable utf8 state");
        }
    } else {
        assert(false && "unreachable escape state");
    }
}

void terminal_context::Worker() {
    pthread_setname_np(pthread_self(), "terminal worker");

    int temp = 0;
    // poll from fd, and render
    struct timeval tv;
    while (1) {
        struct pollfd fds[1];
        fds[0].fd = fd;
        fds[0].events = POLLIN;
        int res = poll(fds, 1, 100);

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
                LOG_INFO("Got: %s", hex.c_str());

                // parse output
                pthread_mutex_lock(&lock);
                for (int i = 0; i < r; i++) {
                    Parse(buffer[i]);
                }
                pthread_mutex_unlock(&lock);
            } else if (r < 0 && errno == EIO) {
                // handle child exit
                LOG_INFO("Program exited: %ld %d", r, errno);
                // relaunch
                pthread_mutex_lock(&lock);
                close(fd);
                fd = -1;

                // print message in a separate line
                if (col > 0) {
                    row += 1;
                    DropFirstRowIfOverflow();
                    col = 0;
                }

                std::string message = "[program exited, restarting]";
                for (char ch : message) {
                    InsertUtf8(ch);
                }

                row += 1;
                DropFirstRowIfOverflow();
                col = 0;

                Fork();
                pthread_mutex_unlock(&lock);
                break;
            }
        }

        // check if anything to paste
        std::string paste = GetPaste();
        if (paste.size() > 0) {
            // send OSC 52 ; c ; BASE64 ST
            LOG_INFO("Paste from pasteboard: %s",
                        paste.c_str());
            std::string resp = "\x1b]52;c;" + paste + "\x1b\\";
            WriteFull((uint8_t *)resp.c_str(), resp.size());
        }
    }
    return;
}

// fork & create pty
// assume lock is held
void terminal_context::Fork() {
    struct winsize ws = {};
    ws.ws_col = num_cols;
    ws.ws_row = num_rows;

    int pid = forkpty(&fd, nullptr, nullptr, &ws);
    if (!pid) {
#ifdef STANDALONE
        execl("/bin/bash", "/bin/bash", nullptr);
#else
        // override HOME to /storage/Users/currentUser since it is writable
        const char *home = "/storage/Users/currentUser";
        setenv("HOME", home, 1);
        setenv("PWD", home, 1);
        // set LD_LIRBARY_PATH for shared libraries
        setenv("LD_LIBRARY_PATH", "/data/app/base.org/base_1.0/lib", 1);
        // override TMPDIR for tmux
        setenv("TMUX_TMPDIR", "/data/storage/el2/base/cache", 1);
        chdir(home);
        //execl("/data/app/bin/bash", "/data/app/bin/bash", nullptr);
        execl("/bin/sh", "/bin/sh", nullptr);
#endif
    }

    // set as non blocking
    int res = fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    assert(res == 0);

    // start terminal worker in another thread
    pthread_t terminal_thread;
    pthread_create(&terminal_thread, NULL, TerminalWorker, this);
}

static terminal_context term;

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

// glyph info
struct character {
    // location within the large texture
    float left;
    float right;
    float top;
    float bottom;
    // x, y offset from origin for bearing etc.
    int xoff;
    int yoff;
    // glyph size
    int width;
    int height;
};

// record info for each character
// map from (codepoint, font weight) to character
static std::map<std::pair<uint32_t, enum font_weight>, struct character> characters;
// code points to load from the font
static std::set<uint32_t> codepoints_to_load;
// do we need to reload font due to missing glyphs?
static bool need_rebuild_atlas = false;
// id of texture for glyphs
static GLuint atlas_texture_id;
// there is a limit on how big a texture can be
static int atlas_width = 8192;

static void ResizeTo(int new_term_row, int new_term_col, bool update_viewport = true) {
    // update viewport
    if (update_viewport) {
        vw100 = new_term_col * font_width;
        vh100 = new_term_row * font_height;
    }

    term.ResizeTo(new_term_row, new_term_col);
}

void Start() {
    pthread_mutex_lock(&term.lock);
    if (term.fd != -1) {
        return;
    }

    // setup terminal, default to 80x24
    term.ResizeTo(24, 80);

    term.Fork();

    pthread_mutex_unlock(&term.lock);
}

void SendData(uint8_t *data, size_t length) {
    if (term.fd == -1) {
        return;
    }

    // reset scroll offset to bottom
    scroll_offset = 0.0;

    term.WriteFull(data, length);
}

// load font
// texture contains all glyphs of all weights:
// fixed width of max_font_width, variable height based on face->glyph->bitmap.rows
// glyph goes in vertical, possibly not filling the whole row space:
//    0.0       1.0
// 0.0 +------+--+
//     | 0x00 |  |
// 0.5 +------+--+
//     | 0x01    |
// 1.0 +------+--+
static void BuildFontAtlas() {
    need_rebuild_atlas = false;

    FT_Library ft;
    FT_Error err = FT_Init_FreeType(&ft);
    assert(err == 0);

    decltype(characters) newChars;
    
    std::vector<struct font_spec> fonts = {
#ifdef STANDALONE
        {"/usr/share/fonts/noto/NotoSansMono-Regular.ttf", {font_weight::regular}},
        {"/usr/share/fonts/noto/NotoSansMono-Bold.ttf", {font_weight::bold}},
        {"/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc", {.weight=font_weight::regular, .ttc_index=0}}
#else
        {
            "/system/fonts/NotoSansMono[wdth,wght].ttf",
            {.variable_width = 88 << 16, .variable_weight = 400 << 16}
        },
        {
            "/system/fonts/NotoSansMono[wdth,wght].ttf",
            {.weight = font_weight::bold, .variable_width = 88 << 16, .variable_weight = 700 << 16}
        },
        {"/system/fonts/NotoSansCJK-Regular.ttc", {.ttc_index=2}} // 0=JP, 1=KR
#endif
    };

    // save glyph for all characters of all weights
    // only one channel
    int bound = font_height, num_rows = 1, row_pointer = 0;
    std::vector<uint8_t> bitmap(bound * atlas_width, 0);

    for (const auto & fnt : fonts) {
        const char *font = fnt.path;
        FT_Face face;
        err = FT_New_Face(ft, font, fnt.opts.ttc_index, &face);
        assert(err == 0);
    
        if (fnt.opts.variable_width.has_value() || fnt.opts.variable_weight.has_value()) {
            FT_MM_Var * vars;
            std::vector<FT_Fixed> coords;
            if (FT_Get_MM_Var(face, &vars) == 0) {
                for (int ia = 0; ia < vars->num_axis; ia++) {
                    if (strcmp("wdth", vars->axis[ia].name) == 0)
                        coords.push_back(fnt.opts.variable_width.value_or(vars->axis[ia].def));
                    else if (strcmp("wght", vars->axis[ia].name) == 0)
                        coords.push_back(fnt.opts.variable_weight.value_or(vars->axis[ia].def));
                    else
                        coords.push_back(vars->axis[ia].def);
                }
                FT_Set_Var_Design_Coordinates(face, coords.size(), coords.data());
                FT_Done_MM_Var(ft, vars);
            }
        }
        
        FT_Set_Pixel_Sizes(face, 0, font_height);
        // Note: in 26.6 fractional pixel format
        LOG_INFO(
                    "Ascender: %d Descender: %d Height: %d XMin: %ld XMax: %ld "
                    "YMin: %ld YMax: %ld XScale: %ld YScale: %ld",
                    face->ascender, face->descender, face->height, face->bbox.xMin, face->bbox.xMax, face->bbox.yMin,
                    face->bbox.yMax, face->size->metrics.x_scale, face->size->metrics.y_scale);
        
        for (auto charCode : codepoints_to_load) {
            // already loaded
            if (newChars.count({charCode, fnt.opts.weight}))
                continue;
            FT_ULong glyphIndex = FT_Get_Char_Index(face, charCode);
            // allow NUL to be loaded
            if (!charCode || glyphIndex) {
                FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER);
            }
            else {
                if (&fnt == &fonts.back()) {
                    newChars[{charCode, font_weight::regular}] = newChars[{0, font_weight::regular}];
                }
                continue;
            }

            LOG_INFO(
                        "Weight: %d Char: %d(0x%x) Glyph: %d %d Left: "
                        "%d "
                        "Top: %d "
                        "Advance: %ld",
                        fnt.weight, charCode, charCode, face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap_left,
                        face->glyph->bitmap_top, face->glyph->advance.x);

            auto glyph = face->glyph;
            auto bits = face->glyph->bitmap;

            int row_start;
            // if current row can't fit new char, extend a row
            if (row_pointer + bits.width < atlas_width) {
                row_start = row_pointer;
                row_pointer += bits.width;
            } else {
                row_start = 0;
                row_pointer = bits.width;
                // a new row cannot be added
                if (bound * (num_rows + 1) > atlas_width)
                    break;
                num_rows += 1;
                bitmap.resize(bound * atlas_width * num_rows);
            }
            int col_start = bound * (num_rows - 1);

            for (int i = 0; i < bits.rows; i++) {
                for (int j = 0; j < bits.width; j++) {
                    bitmap[
                        atlas_width * (col_start + i)
                        + row_start + j
                    ] =
                    bits.buffer[bits.width * i + j];
                }
            }

            // compute location within the texture
            // first pass: store pixels
            character character = {
                .left = float(row_start),
                .right = float(row_start + bits.width - 1),
                .top = float(col_start),
                .bottom = float(col_start + bits.rows - 1),
                .xoff = glyph->bitmap_left,
                .yoff = int(baseline_height + glyph->bitmap_top - bits.rows),
                .width = int(bits.width),
                .height = int(bits.rows),
            };
            newChars.insert(std::make_pair(std::make_pair(charCode, fnt.weight), character));
        }


        FT_Done_Face(face);
    }

    FT_Done_FreeType(ft);

    int atlas_height = bound * num_rows;
    // now bitmap contains all glyphs
    // second pass: convert pixels to uv coordinates
    for (auto &pair : newChars) {
        auto& g = pair.second;
        // https://stackoverflow.com/questions/35454432/finding-image-pixel-coordinates-integers-from-uv-values-floats-of-obj-file
        g.left /= atlas_width;
        g.right /= atlas_width;
        g.top = (g.top + 0.5) / atlas_height;
        g.bottom = (g.bottom + 0.5) / atlas_height;
    }

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    // generate texture
    glBindTexture(GL_TEXTURE_2D, atlas_texture_id);
    {
    GLenum err = glGetError();
    assert(err == GL_NO_ERROR);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas_width, atlas_height, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.data());

    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    characters = newChars;
}

static GLuint program_id;
static GLuint vertex_array;
// vec4 vertex
static GLuint vertex_buffer;
// vec3 textColor
static GLuint text_color_buffer;
// vec3 backGroundColor
static GLuint background_color_buffer;

static void Draw() {
    // blink every 0.5s
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    uint64_t current_msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    // clear buffer with background color
    {
        term_style::color back(predefined_colors[white]);
        glClearColor(back.u.red / 255.0, back.u.green / 255.0, back.u.blue / 255.0, 1.0);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update surface size
    pthread_mutex_lock(&term.lock);
    int aligned_width = vw100 / font_width * font_width;
    int aligned_height = vh100 / font_height * font_height;
    glUniform2f(surface_location, aligned_width, aligned_height);
    glViewport(0, vh100 - aligned_height, aligned_width, aligned_height);

    // set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlas_texture_id);

    // bind our vertex array
    glBindVertexArray(vertex_array);

    int max_lines = vh100 / font_height;
    // vec4 vertex
    static std::vector<GLfloat> vertex_pass0_data;
    static std::vector<GLfloat> vertex_pass1_data;
    // vec3 textColor
    static std::vector<GLfloat> text_color_data;
    // vec3 backgroundColor
    static std::vector<GLfloat> background_color_data;

    vertex_pass0_data.clear();
    vertex_pass0_data.reserve(term.num_rows * term.num_cols * 24);
    vertex_pass1_data.clear();
    vertex_pass1_data.reserve(term.num_rows * term.num_cols * 24);
    text_color_data.clear();
    text_color_data.reserve(term.num_rows * term.num_cols * 18);
    background_color_data.clear();
    background_color_data.reserve(term.num_rows * term.num_cols * 18);

    // ensure at least one line shown, for very large scroll_offset
    int scroll_rows = scroll_offset / font_height;
    if ((int)term.history.size() + max_lines - 1 - scroll_rows < 0) {
        scroll_offset = ((int)term.history.size() + max_lines - 1) * font_height;
        scroll_rows = scroll_offset / font_height;
    }

    for (int i = 0; i < max_lines; i++) {
        // (aligned_height - font_height) is terminal[0] when scroll_offset is zero
        float x = 0.0;
        float y = aligned_height - (i + 1) * font_height;
        int i_row = i - scroll_rows;
        std::vector<term_char> row;
        if (i_row >= 0 && i_row < term.num_rows) {
            row = term.buffer[i_row];
        } else if (i_row < 0 && (int)term.history.size() + i_row >= 0) {
            row = term.history[term.history.size() + i_row];
        } else {
            continue;
        }

        int cur_col = 0;
        for (auto c : row) {
            uint32_t codepoint = c.code;
            auto key = std::pair<uint32_t, enum font_weight>(c.code, c.style.weight);
            auto it = characters.find(key);
            if (it == characters.end())
                it = characters.find(std::make_pair(c.code, font_weight::regular));
            if (it == characters.end()) {
                // reload font to locate it
                LOG_WARN("Missing character: %d of weight %d", c.code, c.style.weight);
                need_rebuild_atlas = true;
                codepoints_to_load.insert(c.code);

                // we don't have the character, fallback to .notdef
                it = characters.find(std::pair<uint32_t, enum font_weight>(0, c.style.weight));
                assert(it != characters.end());
            }

            character ch = it->second;
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

            // pass 0: draw background
            GLfloat g_vertex_pass0_data[24] = {// first triangle: 1->3->4
                                               xpos, ypos + h, 0.0, 0.0, xpos, ypos, 0.0, 0.0, xpos + w, ypos, 0.0, 0.0,
                                               // second triangle: 1->4->2
                                               xpos, ypos + h, 0.0, 0.0, xpos + w, ypos, 0.0, 0.0, xpos + w, ypos + h,
                                               0.0, 0.0};
            vertex_pass0_data.insert(vertex_pass0_data.end(), &g_vertex_pass0_data[0], &g_vertex_pass0_data[24]);

            // pass 1: draw text
            xpos = x + ch.xoff;
            ypos = y + ch.yoff;
            w = ch.width;
            h = ch.height;
            GLfloat g_vertex_pass1_data[24] = {// first triangle: 1->3->4
                                               xpos, ypos + h, ch.left, ch.top, xpos, ypos, ch.left, ch.bottom,
                                               xpos + w, ypos, ch.right, ch.bottom,
                                               // second triangle: 1->4->2
                                               xpos, ypos + h, ch.left, ch.top, xpos + w, ypos, ch.right, ch.bottom,
                                               xpos + w, ypos + h, ch.right, ch.top};
            vertex_pass1_data.insert(vertex_pass1_data.end(), &g_vertex_pass1_data[0], &g_vertex_pass1_data[24]);

            GLfloat g_text_color_buffer_data[18];
            GLfloat g_background_color_buffer_data[18];

            for (int i = 0; i < 6; i++) {
                c.style.fore.put_f3(&g_text_color_buffer_data[i*3]);
                c.style.back.put_f3(&g_background_color_buffer_data[i*3]);
            }

            if ((term.show_cursor && i_row == term.row && cur_col == term.col) ^ term.reverse_video) {
                // invert all colors
                for (int i = 0; i < 18; i++) {
                    g_text_color_buffer_data[i] = 1.0 - g_text_color_buffer_data[i];
                    g_background_color_buffer_data[i] = 1.0 - g_background_color_buffer_data[i];
                }
            }

            // blink: for every 1s, in 0.5s, text color equals to back ground color
            if (c.style.blink && current_msec % 1000 > 500) {
                for (int i = 0; i < 18; i++) {
                    g_text_color_buffer_data[i] = g_background_color_buffer_data[i];
                }
            }

            text_color_data.insert(text_color_data.end(), &g_text_color_buffer_data[0], &g_text_color_buffer_data[18]);
            background_color_data.insert(background_color_data.end(), &g_background_color_buffer_data[0],
                                         &g_background_color_buffer_data[18]);

            x += font_width;
            cur_col++;
        }
    }
    pthread_mutex_unlock(&term.lock);

    // draw in two pass
    glBindBuffer(GL_ARRAY_BUFFER, text_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * text_color_data.size(), text_color_data.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, background_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * background_color_data.size(), background_color_data.data(),
                 GL_STREAM_DRAW);

    // first pass
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertex_pass0_data.size(), vertex_pass0_data.data(), GL_STREAM_DRAW);
    glUniform1i(render_pass_location, 0);
    glDrawArrays(GL_TRIANGLES, 0, vertex_pass0_data.size() / 4);

    // second pass
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertex_pass1_data.size(), vertex_pass1_data.data(), GL_STREAM_DRAW);
    glUniform1i(render_pass_location, 1);
    glDrawArrays(GL_TRIANGLES, 0, vertex_pass1_data.size() / 4);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFlush();
    glFinish();
    AfterDraw();
}


static void *RenderWorker(void *) {
    pthread_setname_np(pthread_self(), "render worker");

    BeforeDraw();

    // build vertex and fragment shader
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    char const *vertex_source = "#version 320 es\n"
                                "\n"
                                "in vec4 vertex;\n"
                                "in vec3 textColor;\n"
                                "in vec3 backgroundColor;\n"
                                "out vec2 texCoords;\n"
                                "out vec3 fragTextColor;\n"
                                "out vec3 fragBackgroundColor;\n"
                                "uniform vec2 surface;\n"
                                "void main() {\n"
                                "  gl_Position.x = vertex.x / surface.x * 2.0f - 1.0f;\n"
                                "  gl_Position.y = vertex.y / surface.y * 2.0f - 1.0f;\n"
                                "  gl_Position.z = 0.0;\n"
                                "  gl_Position.w = 1.0;\n"
                                "  texCoords = vertex.zw;\n"
                                "  fragTextColor = textColor;\n"
                                "  fragBackgroundColor = backgroundColor;\n"
                                "}";
    glShaderSource(vertex_shader_id, 1, &vertex_source, NULL);
    glCompileShader(vertex_shader_id);

    int info_log_length;
    glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        std::vector<char> vertex_shader_error_message(info_log_length + 1);
        glGetShaderInfoLog(vertex_shader_id, info_log_length, NULL, &vertex_shader_error_message[0]);
        LOG_ERROR("Failed to build vertex shader: %s", &vertex_shader_error_message[0]);
    }

    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    char const *fragment_source = "#version 320 es\n"
                                  "\n"
                                  "precision lowp float;\n"
                                  "in vec2 texCoords;\n"
                                  "in vec3 fragTextColor;\n"
                                  "in vec3 fragBackgroundColor;\n"
                                  "out vec4 color;\n"
                                  "uniform sampler2D text;\n"
                                  "uniform int renderPass;\n"
                                  "void main() {\n"
                                  "  if (renderPass == 0) {\n"
                                  "    color = vec4(fragBackgroundColor, 1.0);\n"
                                  "  } else {\n"
                                  "    float alpha = texture(text, texCoords).r;\n"
                                  "    color = vec4(fragTextColor, 1.0) * alpha;\n"
                                  "  }\n"
                                  "}";
    // blending is done by opengl (GL_ONE + GL_ONE_MINUS_SRC_ALPHA):
    // final = src * 1 + dest * (1 - src.a)
    // first pass: src = (fragBackgroundColor, 1.0), dest = (1.0, 1.0, 1.0, 1.0), final = (fragBackgroundColor, 1.0)
    // second pass: src = (fragTextColor * alpha, alpha), dest = (fragBackgroundColor, 1.0), final = (fragTextColor *
    // alpha + fragBackgroundColor * (1 - alpha), 1.0)
    glShaderSource(fragment_shader_id, 1, &fragment_source, NULL);
    glCompileShader(fragment_shader_id);

    glGetShaderiv(fragment_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        std::vector<char> fragment_shader_error_message(info_log_length + 1);
        glGetShaderInfoLog(fragment_shader_id, info_log_length, NULL, &fragment_shader_error_message[0]);
        LOG_ERROR("Failed to build fragment shader: %s", &fragment_shader_error_message[0]);
    }

    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);

    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        std::vector<char> link_program_error_message(info_log_length + 1);
        glGetProgramInfoLog(program_id, info_log_length, NULL, &link_program_error_message[0]);
        LOG_ERROR("Failed to link program: %s", &link_program_error_message[0]);
    }

    surface_location = glGetUniformLocation(program_id, "surface");
    assert(surface_location != -1);

    render_pass_location = glGetUniformLocation(program_id, "renderPass");
    assert(render_pass_location != -1);

    glUseProgram(program_id);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    // load font from ttf for the initial characters
    glGenTextures(1, &atlas_texture_id);
    // load common characters initially
    codepoints_to_load.insert(0);
    for (uint32_t i = 32; i < 128; i++) {
        codepoints_to_load.insert(i);
    }
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &atlas_width);
    BuildFontAtlas();

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
            //LOG_INFO("FPS: %d, %ld ms per draw", fps, sum / fps);
            fps = 0;
            time.clear();
        }

        if (need_rebuild_atlas) {
            BuildFontAtlas();
        }
    }
}


// on resize
void Resize(int new_width, int new_height) {
    pthread_mutex_lock(&term.lock);
    vw100 = new_width;
    vh100 = new_height;

    ResizeTo(vh100 / font_height, vw100 / font_width, false);
    pthread_mutex_unlock(&term.lock);
}

// handle scrolling
void ScrollBy(double offset) {
    pthread_mutex_lock(&term.lock);
    // natural scrolling
    scroll_offset -= offset;
    if (scroll_offset < 0) {
        scroll_offset = 0.0;
    }
    pthread_mutex_unlock(&term.lock);
}

// start render thread
void StartRender() {
    pthread_t render_thread;
    pthread_create(&render_thread, NULL, RenderWorker, NULL);
}

#ifdef STANDALONE
#include <GLFW/glfw3.h>
static GLFWwindow *window;

void BeforeDraw() {
    glfwMakeContextCurrent(window);
}

void AfterDraw() {
    glfwSwapBuffers(window);
}

void KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                  int mode) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_ENTER) {
            uint8_t data[] = {0x0d};
            SendData(data, 1);
        } else if (key == GLFW_KEY_BACKSPACE) {
            uint8_t data[] = {0x7f};
            SendData(data, 1);
        } else if (key == GLFW_KEY_UP) {
            uint8_t data[] = {0x1b, 0x5b, 0x41};
            SendData(data, 3);
        } else if (key == GLFW_KEY_DOWN) {
            uint8_t data[] = {0x1b, 0x5b, 0x42};
            SendData(data, 3);
        } else if (key == GLFW_KEY_RIGHT) {
            uint8_t data[] = {0x1b, 0x5b, 0x43};
            SendData(data, 3);
        } else if (key == GLFW_KEY_LEFT) {
            uint8_t data[] = {0x1b, 0x5b, 0x44};
            SendData(data, 3);
        } else if (key == GLFW_KEY_TAB) {
            uint8_t data[] = {0x09};
            SendData(data, 1);
        }
    }
}

void CharCallback(GLFWwindow* window, uint32_t codepoint) {
    // TODO: encode utf8 properly
    SendData((uint8_t *)&codepoint, 1);
}


void Copy(std::string base64) {
    // TODO
}

void RequestPaste() {
    // TODO
}

std::string GetPaste() {
    // TODO
    return "";
}

#ifdef TESTING

void ResizeWidth(int new_width) {
    // Do nothing
}

#else
void ResizeWidth(int new_width) {
    int current_width, current_height;
    glfwGetWindowSize(window, &current_width, &current_height);
    glfwSetWindowSize(window, new_width, current_height);
}

int main() {
    // Init GLFW
    glfwInit();

    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    int window_width = 80 * font_width;
    int window_height = 30 * font_height;
    window =
        glfwCreateWindow(window_width, window_height, "Terminal", nullptr, nullptr);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCharCallback(window, CharCallback);

    Start();
    StartRender();
    Resize(window_width, window_height);
    while (!glfwWindowShouldClose(window)) {
        // Check if any events have been activated (key pressed, mouse moved etc.)
        // and call corresponding response functions
        glfwPollEvents();
    }
}
#endif
#endif
