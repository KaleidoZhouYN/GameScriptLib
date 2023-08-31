#include "win.h"
#include "controller.h"
#include <iostream>

HWND GetHWnd(const std::string& keyword)
{
	// 获取目标进程ID
	WinInfo_t winInfo;
	GetHWndByName(keyword.c_str(), reinterpret_cast<LPARAM>(&winInfo));
	std::cout << "找到相应的程序，其标题为" << std::endl << winInfo.second << std::endl;

	WinTitleList_t childList;
	GetHWndChild(winInfo.first, reinterpret_cast<LPARAM>(&childList));
	auto child = childList[0];
	std::cout << "找到子程序，其标题为" << std::endl << child.second << std::endl;
	HWND hWnd = child.first;
	return hWnd;
}

void move_to_next(Controller* pc)
{
	float TargetX_p = 1714.00 / 1920.0;
	float TargetY_p = 530.0 / 1080;
	POINT HW = pc->get_window_HW();
	int TargetX = static_cast<int>(TargetX_p * HW.x);
	int TargetY = static_cast<int>(TargetY_p * HW.y);
	pc->mouse_lclick(TargetX, TargetY, 0.1);
}

int main() {
	std::string keyword = "雷電模擬器";
	HWND hWnd = GetHWnd(keyword);
	Controller controller(hWnd);
	for (int i = 0; i < 20; i++)
	{
		move_to_next(&controller);
		Sleep(1000);
	}
	return 0; 
}