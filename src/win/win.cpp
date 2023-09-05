
#include "win.h"

/**
* @param: hWnd ���ھ��
* @callback: lParam Long_ptr���ͣ���ʵ�ʴ���Ϊ std::vector<std::string>*, ��ʾwindowsTitle_Lists
* @return: BOOL �Ƿ�ɹ�
*/

BOOL CALLBACK EnumWindowsTitleProc(HWND hWnd, LPARAM lParam)
{
	// convert lParam back to vector<string>
	WinTitleList_t& windowTitle_List = *(reinterpret_cast<WinTitleList_t*>(lParam));
	auto length = GetWindowTextLengthA(hWnd);
	char* buffer = new char[length + 1];
	GetWindowTextA(hWnd, buffer, length + 1);
	if (IsWindowVisible(hWnd) && length != 0)
	{
		std::string strTo(buffer);

		windowTitle_List.push_back({ hWnd,strTo });
	}

	delete[] buffer;
	return TRUE;
}

/*
* @brief: ��ȡ���������һ������keyword��hWnd
* @param: keyword ���ڱ���(������
* @param: lParam ��Ӧ��winInfo
* @return: ��Ӧ��HWND
*/
BOOL GetHWndByKeyword(const char* keyword, LPARAM lParam)
{
	WinTitleList_t windowTitle_List;
	EnumWindows(&EnumWindowsTitleProc, reinterpret_cast<LPARAM>(&windowTitle_List));

	WinInfo_t& winInfo = *(reinterpret_cast<WinInfo_t*>(lParam));
	for (auto windowTitle : windowTitle_List)
	{
		if (windowTitle.second.find(keyword) != std::string::npos) {
			winInfo = windowTitle;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL GetHWndByName(const char* name, LPARAM lParam)
{
	WinTitleList_t windowTitle_List;
	EnumWindows(&EnumWindowsTitleProc, reinterpret_cast<LPARAM>(&windowTitle_List));

	WinInfo_t& winInfo = *(reinterpret_cast<WinInfo_t*>(lParam));
	for (auto windowTitle : windowTitle_List)
	{
		if (windowTitle.second == std::string(name)) {
			winInfo = windowTitle;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL GetHWndListByName(const char* keyword, LPARAM lParam)
{
	WinTitleList_t windowTitle_List_All;
	EnumWindows(&EnumWindowsTitleProc, reinterpret_cast<LPARAM>(&windowTitle_List_All));

	WinTitleList_t& windowTitle_List = *(reinterpret_cast<WinTitleList_t*>(lParam));
	// ��ȡ�������ж�Ӧ��
	for (auto windowTitle : windowTitle_List_All)
	{
		if (windowTitle.second.find(keyword) != std::string::npos) {
			windowTitle_List.push_back(windowTitle);
		}
	}
	return TRUE;
}

BOOL GetHWndChild(HWND hWnd, LPARAM lParam)
{
	return EnumChildWindows(hWnd, &EnumWindowsTitleProc, lParam);
}

#include<deque>
BOOL GetHWndAllChild(HWND hWnd,LPARAM lParam)
{
	WinTitleList_t& windowTitleList = *(reinterpret_cast<WinTitleList_t*>(lParam));
	std::deque<WinInfo_t> winQueue; 

	auto length = GetWindowTextLengthA(hWnd);
	char* buffer = new char[length + 1];
	GetWindowTextA(hWnd, buffer, length + 1);
	std::string strTo(buffer);

	winQueue.push_back(WinInfo_t(hWnd, strTo));
	windowTitleList.push_back(WinInfo_t(hWnd, strTo));
	while (!winQueue.empty())
	{
		auto f = winQueue.front();
		winQueue.pop_front(); 
		WinTitleList_t temp; 
		GetHWndChild(f.first, reinterpret_cast<LPARAM>(&temp));
		winQueue.insert(winQueue.end(), temp.begin(), temp.end());
		windowTitleList.insert(windowTitleList.end(), temp.begin(), temp.end());
	}
	return TRUE; 
}

BOOL FindHWndRedraw(HWND hWnd, LPARAM lParam)
{
	WinTitleList_t temp; 
	GetHWndAllChild(hWnd, reinterpret_cast<LPARAM>(&temp));

	WinInfo_t& result = *(reinterpret_cast<WinInfo_t*>(lParam));
	for (auto it = temp.begin(); it != temp.end(); it++)
	{
		HWND childWnd = it->first; 
		DWORD classStyle = GetClassLong(childWnd, GCL_STYLE);
		if (classStyle & CS_VREDRAW)
		{
			result = *it; 
			return TRUE;
		}
	}
	return FALSE; 
}

BOOL GetActuallWindowRect(HWND hwnd, LPRECT lprect)
{
	// ��ȡ���ڵ�DPI
	UINT dpi = GetDpiForWindow(hwnd);
	float scale = static_cast<float>(dpi) / STD_DPI;

	// ��ȡ���ڴ�С
	RECT windowRect;
	GetWindowRect(hwnd, &windowRect);

	lprect->left = static_cast<int>(windowRect.left * scale);
	lprect->right = static_cast<int>(windowRect.right * scale);
	lprect->top = static_cast<int>(windowRect.top * scale);
	lprect->bottom = static_cast<int>(windowRect.bottom * scale);

	return 0; 
}