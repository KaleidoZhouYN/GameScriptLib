#include "screenshot.h"
#include<iostream>

int main() {
	const char* keyword = "雷电";
	WinInfo_t winInfo; 
	GetHWndByName(keyword, reinterpret_cast<LPARAM>(&winInfo));
	std::cout << "找到相应的程序，其标题为" << std::endl << winInfo.second << std::endl;

	WinTitleList_t winChildList; 
	GetHWndChild(winInfo.first, reinterpret_cast<LPARAM>(&winChildList)); 
	auto firstChild = winChildList[0];

	std::cout << "找到相应的子程序，其标题为" << std::endl << firstChild.second << std::endl; 

	winChildList.clear();
	GetHWndChild(firstChild.first, reinterpret_cast<LPARAM>(&winChildList));
	firstChild = winChildList[0];

	std::cout << "找到相应的子程序，其标题为" << std::endl << firstChild.second << std::endl;

	const char* savePath = "s.jpg";
	CaptureWindowScreenshotCPU(winInfo.first, savePath);
	return 0; 
}