// build with:
// g++ test.cpp terminal.cpp -I/usr/include/freetype2 -DSTANDALONE -DTESTING -o test -lCatch2Main -lCatch2 -lfreetype -lGLESv2 -lglfw
#include "terminal.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE( "Columns change with input", "" ) {
    terminal_context ctx;

    ctx.ResizeTo(24, 80);
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );
    ctx.Parse('a');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 1 );
}

TEST_CASE( "Line feed", "" ) {
    terminal_context ctx;

    ctx.ResizeTo(2, 80);
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );

    // a|
    ctx.Parse('a');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 1 );

    ctx.Parse('\x0d');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );

    // a
    // |
    ctx.Parse('\x0a');
    REQUIRE( ctx.row == 1 );
    REQUIRE( ctx.col == 0 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );

    // a
    // b|
    ctx.Parse('b');
    REQUIRE( ctx.row == 1 );
    REQUIRE( ctx.col == 1 );
    REQUIRE( ctx.terminal[1][0].ch == 'b' );

    // b
    //  |
    ctx.Parse('\x0a');
    REQUIRE( ctx.row == 1 );
    REQUIRE( ctx.col == 1 );
    REQUIRE( ctx.terminal[0][0].ch == 'b' );
    REQUIRE( ctx.terminal[1][0].ch == ' ' );
}

TEST_CASE( "Backspace", "" ) {
    terminal_context ctx;

    ctx.ResizeTo(24, 80);
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );

    // a|
    ctx.Parse('a');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 1 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );

    ctx.Parse('\x08');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );

    // b|
    ctx.Parse('b');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 1 );
    REQUIRE( ctx.terminal[0][0].ch == 'b' );
}

TEST_CASE( "Tab", "" ) {
    terminal_context ctx;

    ctx.ResizeTo(24, 80);
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );

    // a|
    ctx.Parse('a');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 1 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );

    // a       |
    ctx.Parse('\x09');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 8 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );

    // a       b|
    ctx.Parse('b');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 9 );
    REQUIRE( ctx.terminal[0][8].ch == 'b' );
}

TEST_CASE( "Insert Characters", "" ) {
    terminal_context ctx;

    ctx.ResizeTo(24, 80);
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );

    // a|
    ctx.Parse('a');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 1 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );

    // a
    ctx.Parse('\x0d');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );

    // |a
    // CSI @
    ctx.Parse('\x1b');
    ctx.Parse('[');
    ctx.Parse('@');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );
    REQUIRE( ctx.terminal[0][0].ch == ' ' );
    REQUIRE( ctx.terminal[0][1].ch == 'a' );

    // |  a
    // CSI 2 @
    ctx.Parse('\x1b');
    ctx.Parse('[');
    ctx.Parse('2');
    ctx.Parse('@');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );
    REQUIRE( ctx.terminal[0][0].ch == ' ' );
    REQUIRE( ctx.terminal[0][1].ch == ' ' );
    REQUIRE( ctx.terminal[0][2].ch == ' ' );
    REQUIRE( ctx.terminal[0][3].ch == 'a' );
}

TEST_CASE( "Cursor Up", "" ) {
    terminal_context ctx;

    ctx.ResizeTo(24, 80);
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );

    // a|
    ctx.Parse('a');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 1 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );

    // a|
    ctx.Parse('\x0d');
    ctx.Parse('\x1b');
    ctx.Parse('[');
    ctx.Parse('A');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );

    // a
    //  |
    ctx.Parse('\x0a');
    REQUIRE( ctx.row == 1 );
    REQUIRE( ctx.col == 0 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );

    // a
    // |
    ctx.Parse('\x0b');
    REQUIRE( ctx.row == 1 );
    REQUIRE( ctx.col == 0 );

    // a
    // CSI A
    ctx.Parse('\x0d');
    ctx.Parse('\x1b');
    ctx.Parse('[');
    ctx.Parse('A');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );

    // b|
    ctx.Parse('b');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 1 );
    REQUIRE( ctx.terminal[0][0].ch == 'b' );

    // b
    // |
    // set scroll margin [2,3]
    ctx.Parse('b');
    ctx.Parse('\x0d');
    ctx.Parse('\x1b');
    ctx.Parse('[');
    ctx.Parse('2');
    ctx.Parse(';');
    ctx.Parse('3');
    ctx.Parse('r');
    REQUIRE( ctx.row == 1 );
    REQUIRE( ctx.col == 0 );

    // b
    // |
    // CSI A
    ctx.Parse('\x0d');
    ctx.Parse('\x1b');
    ctx.Parse('[');
    ctx.Parse('A');
    REQUIRE( ctx.row == 1 );
    REQUIRE( ctx.col == 0 );
}

TEST_CASE( "Erase in Display", "" ) {
    terminal_context ctx;

    ctx.ResizeTo(24, 80);
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );

    // a|
    ctx.Parse('a');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 1 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );

    // ab|
    ctx.Parse('b');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 2 );
    REQUIRE( ctx.terminal[0][1].ch == 'b' );

    // ab
    ctx.Parse('\x08');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 1 );

    // a|
    // CSI J
    ctx.Parse('\x1b');
    ctx.Parse('[');
    ctx.Parse('J');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 1 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );
    REQUIRE( ctx.terminal[0][1].ch == ' ' );
}

TEST_CASE( "Save Cursor", "" ) {
    terminal_context ctx;

    ctx.ResizeTo(24, 80);
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );

    // a|
    ctx.Parse('a');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 1 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );

    // a|
    ctx.Parse('\x1b');
    ctx.Parse('7');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 1 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );

    // a
    ctx.Parse('\x08');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );

    // a|
    ctx.Parse('\x1b');
    ctx.Parse('8');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 1 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );

    // ab|
    ctx.Parse('b');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 2 );
    REQUIRE( ctx.terminal[0][1].ch == 'b' );
}

TEST_CASE( "Tab Clear", "" ) {
    terminal_context ctx;

    ctx.ResizeTo(24, 80);
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );

    // a|
    ctx.Parse('a');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 1 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );

    // a       |
    ctx.Parse('\x09');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 8 );
    REQUIRE( ctx.terminal[0][0].ch == 'a' );

    // a       b|
    ctx.Parse('b');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 9 );
    REQUIRE( ctx.terminal[0][8].ch == 'b' );

    // a       b|
    ctx.Parse('\x1b');
    ctx.Parse('[');
    ctx.Parse('3');
    ctx.Parse('g');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 9 );

    // got to last column
    ctx.Parse('\x09');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 79 );
}