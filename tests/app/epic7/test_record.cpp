#include "capture_gl.h"
#include "controller.h"
#include "win.h"
#include "easylogging++.h"
#define ELPP_THREAD_SAFE
#include "screenshot.h"
#include <iostream>

HWND GetControlHWnd(const std::string& keyword)
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

HWND GetRedrawHWnd(const std::string& keyword)
{
	// ��ȡĿ�����ID
	WinInfo_t winInfo;
	GetHWndByName(keyword.c_str(), reinterpret_cast<LPARAM>(&winInfo));
	std::cout << "�ҵ���Ӧ�ĳ��������Ϊ" << std::endl << winInfo.second << std::endl;

	FindHWndRedraw(winInfo.first, reinterpret_cast<LPARAM>(&winInfo));
	std::cout << "�ҵ�VREDRAW�ӳ��������Ϊ" << std::endl << winInfo.second << std::endl;
	HWND hWnd = winInfo.first;
	return hWnd;
}


/*
cv::Mat convert2Mat(FrameInfo frame)
{
	cv::Mat img(frame.header.width, frame.header.height, CV_8UC3, frame.buffer);
	return img; 
}
*/

void save_img(FrameInfo frame, const int index)
{
	std::string path = "C:\\Users\\zhouy\\Desktop\\test\\";
	std::string name = std::to_string(index) + ".bmp";
	path += name;
	SaveToBMP(frame.buffer, frame.header.width, frame.header.height, path.c_str());
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

INITIALIZE_EASYLOGGINGPP

int main()
{
	el::Configurations conf;
	conf.setToDefault();
	conf.set(el::Level::Global, el::ConfigurationType::Filename, R"(test_record.log)");  // ������־�ļ���·��
	el::Loggers::reconfigureLogger("default", conf); // ��������Ĭ�ϵ� logger

	std::string keyword = "���ģ�M��";
	HWND hWnd = GetControlHWnd(keyword);
	Controller* controller = new Controller(hWnd);

	hWnd = GetRedrawHWnd(keyword);
	const char* dllPath = R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\src\inject\OpenGLScreenShotDLL.dll)";
	const size_t MaxShmSize = 2560 * 1600 * 3;
	OpenglCapture* capture = new OpenglCapture(hWnd, dllPath, MaxShmSize);
	capture->start(); 

	// ʵ�ֿ����߼�
	for (int i = 0; i < 20; i++)
	{
		FrameInfo frame;
		capture->capture_frame(&frame);
		save_img(frame, i);
		move_to_next(controller);
		Sleep(1 * 1000);
		std::cout << "operate " << i << " times" << std::endl;
	}
	return 0;

}
