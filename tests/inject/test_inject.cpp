#include "win.h"
#include <iostream>
#include <string>
#include "shm_data.h"
#include "opencv2/opencv.hpp"
#include "hook.h"
#include "injector.h"
#include <signal.h>
#include <mutex>

// multi-thread mutex
std::mutex mtx; 

// singleton static variable define
std::map<std::string, MutexSingleton*> MutexSingleton::_singleton = {};
MutexSingleton::GarbageCollector MutexSingleton::gc;
std::map<std::string, SharedMemorySingleton*> SharedMemorySingleton::_singleton = {};
SharedMemorySingleton::GarbageCollector SharedMemorySingleton::gc;
 
const std::string shm_name = "CaptureBuffer_";
const int MaxShmSize = 2560 * 1600 * 3;

BOOL ReadBufferAndShow(ScreenShotHook* sh, const std::string& name_)
{
	cv::namedWindow(name_, cv::WINDOW_AUTOSIZE);
	BYTE* pixels = nullptr;
	while (TRUE) {
		// 读共享内存，加锁
		Lock lock(sh->mutex);

		// 读入header
		SharedDataHeader shm_header;

		// 首先保存frame
		memcpy(&shm_header, sh->shm->data<BYTE*>(), sizeof(shm_header));
		int width = shm_header.width, height = shm_header.height, channel = shm_header.channel;
		if (width * height * channel > 0)
		{
			pixels = new BYTE[width * height * channel];

			memcpy(pixels, sh->shm->data<BYTE*>() + sizeof(shm_header), width * height * channel);
		}
		lock.unlock();
		

		// 转成opencv可以接受的格式
		std::unique_lock<std::mutex> dis_lock(mtx);
		if (pixels)
		{
			cv::Mat frame(height, width, CV_8UC3, pixels);
			// 上下翻转，因为openGL的起点是左下角
			cv::flip(frame, frame, 0);
			cv::resize(frame, frame, cv::Size(400, 225));
			//cv::imwrite(R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\tests\inject\screenshot_SM.bmp)", frame);
			cv::imshow(name_, frame);
			delete[] pixels;
		}
		pixels = nullptr;
		cv::waitKey(25);
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


void threadFunction(const std::string& keyword) {
	const char* dllPath = R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\src\inject\OpenGLScreenShotDLL.dll)";

	HWND hWnd = GetHWnd(keyword);
	DWORD processId;
	GetWindowThreadProcessId(hWnd, &processId);

	// injector
	Injector injector(processId);
	std::unique_lock<std::mutex> lock(mtx);
	injector.inject(dllPath);
	lock.unlock(); 

	// 读取buffer并显示

	std::stringstream ss;
	ss << shm_name << processId;
	ScreenShotHook sh(ss.str(), MaxShmSize);
	sh.start();

	ReadBufferAndShow(&sh, ss.str());
	// 清理并关闭句柄

	injector.release();
}

void signalHandler(int signum) {

	exit(signum);  // 终止程序
}

int main() {
	signal(SIGINT, signalHandler);

	std::thread t1(threadFunction, "雷电模拟器");
	std::thread t2(threadFunction,"雷电模拟器-1");
	t1.join();
	t2.join(); 
	return 0; 
}