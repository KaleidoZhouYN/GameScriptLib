#include "win.h"
#include "controller.h"
#include <iostream>

HWND GetHWnd(const std::string& keyword)
{
	// ��ȡĿ�����ID
	WinInfo_t winInfo;
	GetHWndByName(keyword.c_str(), reinterpret_cast<LPARAM>(&winInfo));
	std::cout << "�ҵ���Ӧ�ĳ��������Ϊ" << std::endl << winInfo.second << std::endl;

	WinTitleList_t childList;
	GetHWndChild(winInfo.first, reinterpret_cast<LPARAM>(&childList));
	auto child = childList[0];
	std::cout << "�ҵ��ӳ��������Ϊ" << std::endl << child.second << std::endl;
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
	std::string keyword = "���ģ�M��";
	HWND hWnd = GetHWnd(keyword);
	Controller controller(hWnd);
	for (int i = 0; i < 20; i++)
	{
		move_to_next(&controller);
		Sleep(1000);
	}
	return 0; 
}