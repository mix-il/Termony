#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <stdint.h>
#include <stdlib.h>
#include <string>

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