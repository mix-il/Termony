#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <stdint.h>
#include <stdlib.h>

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

#endif