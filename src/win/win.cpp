
#include "win.h"

/**
* @param: hWnd ���ھ��
* @callback: lParam Long_ptr���ͣ���ʵ�ʴ���Ϊ std::vector<std::string>*, ��ʾwindowsTitle_Lists
* @return: BOOL �Ƿ�ɹ�
*/

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
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
* @brief: ��ȡ��Ӧ�����hWnd
* @param: keyword ���ڱ���(������
* @param: lParam ��Ӧ��winInfo
* @return: ��Ӧ��HWND
*/
BOOL GetHWndByName(const char* keyword, LPARAM lParam)
{
	WinTitleList_t windowTitle_List;
	EnumWindows(&EnumWindowsProc, reinterpret_cast<LPARAM>(&windowTitle_List));

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

BOOL GetHWndListByName(const char* keyword, LPARAM lParam)
{
	WinTitleList_t windowTitle_List_All;
	EnumWindows(&EnumWindowsProc, reinterpret_cast<LPARAM>(&windowTitle_List_All));

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
	return EnumChildWindows(hWnd, &EnumWindowsProc, lParam);
}
