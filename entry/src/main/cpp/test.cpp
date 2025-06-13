// build with:
// g++ test.cpp terminal.cpp -I/usr/include/freetype2 -DSTANDALONE -DTESTING -o test -lCatch2Main -lCatch2 -lfreetype -lGLESv2 -lglfw
#include "terminal.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE( "Columns change with input", "" ) {
    terminal_context ctx;

    ctx.ResizeTo(80, 24);
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 0 );
    ctx.Parse('a');
    REQUIRE( ctx.row == 0 );
    REQUIRE( ctx.col == 1 );
}
