#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <pthread.h>
#include <deque>
#include <vector>

// font weight
enum font_weight {
    regular = 0,
    bold = 1,
    NUM_WEIGHT,
};

// maintain terminal status
struct term_style {
    // font weight
    font_weight weight = regular;
    // blinking
    bool blink = false;
    // foreground color
    float fg_red = 0.0;
    float fg_green = 0.0;
    float fg_blue = 0.0;
    // background color
    float bg_red = 1.0;
    float bg_green = 1.0;
    float bg_blue = 1.0;
    term_style();
};

// character in terminal
struct term_char {
    uint32_t ch = ' ';
    term_style style;
};

// escape sequence state machine
enum escape_states {
    state_idle,
    state_esc,
    state_csi,
    state_osc,
    state_dcs,
};

// utf8 decode state machine
enum utf8_states {
    state_initial,
    state_2byte_2,        // expected 2nd byte of 2-byte sequence
    state_3byte_2_e0,     // expected 2nd byte of 3-byte sequence starting with 0xe0
    state_3byte_2_non_e0, // expected 2nd byte of 3-byte sequence starting with non-0xe0
    state_3byte_3,        // expected 3rd byte of 3-byte sequence
    state_4byte_2_f0,     // expected 2nd byte of 4-byte sequence starting with 0xf0
    state_4byte_2_f1_f3,  // expected 2nd byte of 4-byte sequence starting with 0xf1 to 0xf3
    state_4byte_2_f4,     // expected 2nd byte of 4-byte sequence starting with 0xf4
    state_4byte_3,        // expected 3rd byte of 4-byte sequence
    state_4byte_4,        // expected 4th byte of 4-byte sequence
};

struct terminal_context {
    // protect multithreaded usage
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    // pty
    int fd = -1;

    // escape sequence state machine
    escape_states escape_state = state_idle;
    std::string escape_buffer;

    // utf8 decode state machine
    utf8_states utf8_state = state_initial;
    uint32_t current_utf8 = 0;

    // terminal state
    // scrollback history
    std::deque<std::vector<term_char>> history;
    // current text style, see CSI Pm M, SGR
    term_style current_style;

    // DEC private modes
    // DECTCEM, Show cursor
    bool show_cursor = true;
    // DECAWM, Autowrap Mode
    bool autowrap = true;
    // DECSCNM, Reverse Video
    bool reverse_video = false;
    // DECOM, Origin Mode
    bool origin_mode = false;
    // IRM, Insert Mode
    bool insert_mode = false;

    // tab handling
    int tab_size = 8;
    // columns where tab stops
    std::vector<bool> tab_stops;

    // save/restore feature
    int save_row = 0;
    int save_col = 0;
    term_style save_style;

    // terminal content
    std::vector<std::vector<term_char>> terminal;
    // terminal size
    int term_col = 0;
    int term_row = 0;
    // cursor location
    int row = 0;
    int col = 0;

    // DECSTBM, scrolling region
    int scroll_top = 0;
    int scroll_bottom = term_row - 1;

    void ResizeTo(int new_term_row, int new_term_col);

    void DropFirstRowIfOverflow();

    void InsertUtf8(uint32_t codepoint);

    // clamp cursor to valid range
    void ClampCursor();

    // set absolute cursor location
    void SetCursor(int new_row, int new_col);

    // move cursor in relative position
    void MoveCursor(int row_diff, int col_diff);

    // write data to pty until fully sent
    void WriteFull(uint8_t *data, size_t length);

    // handle CSI escape sequences
    void HandleCSI(uint8_t current);

    static void *TerminalWorker(void * data);

    void Parse(uint8_t input);

    void Worker();

    // fork & create pty
    // assume lock is held
    void Fork();
};

// start a terminal
void Start();
// start rendering
void StartRender();
// send data to terminal
void SendData(uint8_t *data, size_t length);
// resize window
void Resize(int width, int height);
void ScrollBy(double offset);

// implemented by code in napi/glfw
extern void BeforeDraw();
extern void AfterDraw();
extern void ResizeWidth(int new_width);
// copy/paste with base64 encoded string
extern void Copy(std::string base64);
extern void RequestPaste();
extern std::string GetPaste();

#endif