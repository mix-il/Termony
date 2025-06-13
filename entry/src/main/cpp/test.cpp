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
}