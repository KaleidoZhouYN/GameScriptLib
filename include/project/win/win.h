#ifndef _WIN_H
#define _WIN_H

// SYSTEM INCLUDES
#include "windows.h"
#include "winuser.h"
#include "wingdi.h"

#include<vector>
#include<string>

using WinInfo_t = std::pair<HWND, std::string>;
using WinTitleList_t = std::vector<WinInfo_t>;

extern "C" BOOL CALLBACK EnumWindowsTitleProc(HWND hWnd, LPARAM lParam);
extern "C" BOOL GetHWndByName(const char* windowName, LPARAM lParam);
extern "C" BOOL GetHWndListByName(const char* keyword, LPARAM lParam);
extern "C" BOOL GetHWndChild(HWND hWnd, LPARAM lParam);
extern "C" BOOL GetHWndAllChild(HWND hWnd, LPARAM lParam);
extern "C" BOOL FindHWndRedraw(HWND hWnd, LPARAM lParam);

#endif // !_WIN_H