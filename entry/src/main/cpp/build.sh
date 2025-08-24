clang++ -std=c++17 -g \
    terminal.cpp -I/usr/include/freetype2 -DSTANDALONE -lfreetype -lutf8proc -lGLESv2 -lglfw \
    -o terminal
