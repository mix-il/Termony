clang++ -std=c++17 -g \
    -DSTANDALONE \
    -I/usr/include/freetype2 -lfreetype -lutf8proc -lGLESv2 -lglfw \
    terminal.cpp -o /tmp/terminal
