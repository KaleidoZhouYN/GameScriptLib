#include "win.h"
#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"
#include <signal.h>
#include <capture_gl.h>

// multi-thread mutex
std::mutex mtx; 


// 2023/08/25
// to do : 增加一个帧率选取，避免帧率过高
BOOL ReadBufferAndShow(OpenglCapture* sh, const std::string& index)
{
	std::string name_ = sh->get_name() + std::string("_") + index;
	cv::namedWindow(name_, cv::WINDOW_AUTOSIZE);
	
	while (TRUE) {
		FrameInfo frame;
		sh->capture_frame(&frame);
		// 转成opencv可以接受的格式
		std::unique_lock<std::mutex> dis_lock(mtx);
		if (frame.buffer_size)
		{
			auto fmt = CV_8UC3; 
			if (frame.header.channel == 4)
				fmt = CV_8UC4; 
			cv::Mat frame(frame.header.height, frame.header.width, fmt, frame.buffer);
			// 上下翻转，因为openGL的起点是左下角
			//cv::flip(frame, frame, 0);
			cv::resize(frame, frame, cv::Size(800, 455));
			cv::imwrite(R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\tests\inject\screenshot_SM.bmp)", frame);
			cv::imshow(name_, frame);
			break;
		}

		if (cv::waitKey(40) == 'q')
			break;
		dis_lock.unlock(); 
	}

	// release resources
	cv::destroyWindow(name_);

	return TRUE; 
}

HWND GetHWnd(const std::string& keyword)
{
	// 获取目标进程ID
	WinInfo_t winInfo;
	GetHWndByName(keyword.c_str(), reinterpret_cast<LPARAM>(&winInfo));
	std::cout << "找到相应的程序，其标题为" << std::endl << winInfo.second << std::endl;

	FindHWndRedraw(winInfo.first, reinterpret_cast<LPARAM>(&winInfo));
	std::cout << "找到VREDRAW子程序，其标题为" << std::endl << winInfo.second << std::endl; 
	HWND hWnd = winInfo.first;
	return hWnd; 
	
}

void PrintLastError()
{
	DWORD error = GetLastError();
	LPVOID errMsg;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		error,
		0, // 默认语言
		(LPSTR)&errMsg,
		0,
		NULL
	);

	std::wcerr << L"Error (" << error << "): " << (LPSTR)errMsg << std::endl;

	LocalFree(errMsg);
}


void threadFunction(OpenglCapture* capture, const std::string& index) {

	// injector

	// 读取buffer并显示
	ReadBufferAndShow(capture, index);
	// 清理并关闭句柄
}

std::shared_ptr<OpenglCapture> capture;

void signalHandler(int signum) {
	capture->end(); 
	exit(signum);  // 终止程序
}

INITIALIZE_EASYLOGGINGPP

int main() {
	signal(SIGINT, signalHandler); 

	el::Configurations conf;
	conf.setToDefault();
	conf.set(el::Level::Global, el::ConfigurationType::Filename, R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\tests\inject\test_inject.log)");  // 设置日志文件的路径
	el::Loggers::reconfigureLogger("default", conf); // 重新配置默认的 logger

	//HWND hwnd = GetHWnd("雷電模擬器");
	HWND hwnd = GetHWnd("幻塔  ");


	//const char* dllPath = R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\src\inject\OpenGLScreenShotDLL.dll)";
	const char* dllPath = R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\src\inject\D3D11ScreenShotDLL.dll)";

	const size_t MaxShmSize = 2560 * 1600 * 3;
	capture = std::make_shared<OpenglCapture>(hwnd, dllPath, MaxShmSize);
	capture->start();


	std::thread t1(threadFunction, capture.get(), "1");
	//std::thread t2(threadFunction, capture.get(), "2");
	t1.join();
	//t2.join(); 
	return 0; 
}