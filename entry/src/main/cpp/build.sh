clang++ -std=c++17 \
    terminal.cpp -I/usr/include/freetype2 -DSTANDALONE -lfreetype -lGLESv2 -lglfw \
    -o terminal