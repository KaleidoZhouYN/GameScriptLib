#ifndef _WIN_SCREENSHOT_H
#define _WIN_SCREENSHOT_H

#include "win.h"
#include <GL/gl.h>
#include <stdio.h>
#include "hook.h"


// DEFINE function
const double STD_DPI = 96.0;
extern "C" int CaptureWindowScreenshotCPU(HWND hWnd, const char* savePath);
extern "C" int CaptureWindowScreenshotOpenGL(HWND hWnd, const char* savePath);
extern "C" void SaveToBMP(const BYTE * data, int width, int height, const char* path);

#endif