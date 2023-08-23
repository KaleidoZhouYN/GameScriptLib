#include "win.h"
#include <iostream>
#include <string>
#include "shm_data.h"
#include "opencv2/opencv.hpp"
#include "screenshot.h"
#include "mutex.h"
#include "shared_memory.h"

// singleton static variable define
std::map<std::string, MutexSingleton*> MutexSingleton::_singleton = {};
MutexSingleton::GarbageCollector MutexSingleton::gc;
std::map<std::string, SharedMemorySingleton*> SharedMemorySingleton::_singleton = {};
SharedMemorySingleton::GarbageCollector SharedMemorySingleton::gc;

bool PathExists(const std::string& s) {
	DWORD dwAttrib = GetFileAttributes(s.c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

SharedMemorySingleton* shm; 
MutexSingleton* mutex; 
const std::string shm_name = "CaptuerBuffer";
const int MaxShmSize = 2560 * 1600 * 3;

BOOL ReadBufferAndShow(void)
{
	cv::namedWindow("Capture", cv::WINDOW_AUTOSIZE);
	BYTE* pixels = nullptr;
	while (TRUE) {
		// 读共享内存，加锁
		Lock lock(mutex);

		// 读入header
		SharedDataHeader shm_header;

		// 首先保存frame
		memcpy(&shm_header, shm->data<BYTE*>(), sizeof(shm_header));
		int width = shm_header.width, height = shm_header.height, channel = shm_header.channel;
		if (width * height * channel > 0)
		{
			pixels = new BYTE[width * height * channel];

			memcpy(pixels, shm->data<BYTE*>() + sizeof(shm_header), width * height * channel);
		}
		lock.unlock();

		// 转成opencv可以接受的格式
		if (pixels)
		{
			cv::Mat frame(height, width, CV_8UC3, pixels);
			// 上下翻转，因为openGL的起点是左下角
			cv::flip(frame, frame, 0);
			cv::resize(frame, frame, cv::Size(800, 450));
			cv::imwrite(R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\tests\inject\screenshot_SM.bmp)", frame);
			cv::imshow("Capture", frame);
			delete[] pixels;

		}
		char c = (char)cv::waitKey(25);
		if (c != -1)
			break;
		pixels = nullptr;
	}

	// release resources
	cv::destroyWindow("Capture");

	return TRUE; 
}

HWND GetLeidian()
{
	// 获取目标进程ID
	const char* keyword = "雷电";
	WinInfo_t winInfo;
	GetHWndByName(keyword, reinterpret_cast<LPARAM>(&winInfo));
	std::cout << "找到相应的程序，其标题为" << std::endl << winInfo.second << std::endl;

	FindHWndRedraw(winInfo.first, reinterpret_cast<LPARAM>(&winInfo));
	std::cout << "找到VREDRAW子程序，其标题为" << std::endl << winInfo.second << std::endl; 
	HWND hWnd = winInfo.first;
	return hWnd; 
}

HWND GetYuanshen()
{
	// 获取目标进程ID
	const char* keyword = "原神";
	WinInfo_t winInfo;
	GetHWndByName(keyword, reinterpret_cast<LPARAM>(&winInfo));
	std::cout << "找到相应的程序，其标题为" << std::endl << winInfo.second << std::endl;

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

int main() {
	const char* dllPath = R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\src\inject\OpenGLScreenShotDLL.dll)";
	if (PathExists(std::string(dllPath)))
		std::cout << "Path exists" << std::endl;
	else
		std::cout << "Path not exist" << std::endl; 
	
	HWND hWnd = GetLeidian(); 
	DWORD processId;
	GetWindowThreadProcessId(hWnd, &processId);

	// 打开目标进程
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
	if (!hProcess) {
		std::cerr << "Error: Could not open process" << std::endl; 
		goto fail; 
	}

	// 在目标进程中分配内存
	void* pDllPath = VirtualAllocEx(hProcess, 0, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	// 写DLL路径到目标进程的内存中
	if (!pDllPath) {
		std::cerr << "Error: Could not Alloc Virtual Memory" << std::endl; 
		goto fail;
	}
	
	if (!WriteProcessMemory(hProcess, pDllPath, dllPath, strlen(dllPath) + 1, 0)) {
		std::cout << "Error: Could not write dll path" << std::endl; 
		goto fail; 
	}

	//在目标进程中启动一个新线程来加载DLL
	HANDLE hThread = CreateRemoteThreadEx(hProcess, 0, 0,
		(LPTHREAD_START_ROUTINE)LoadLibraryA,
		pDllPath, 0, 0, 0);


	// 读取buffer并显示

	char t; 
	std::cin >> t; 

	shm = SharedMemorySingleton::Instance(shm_name, MaxShmSize);
	shm->open(); 
	mutex = MutexSingleton::Instance(shm_name+"Mutex");
	mutex->open(); 

	ReadBufferAndShow(); 
	// 清理并关闭句柄


fail:
	PrintLastError(); 
	if (hThread)
		CloseHandle(hThread);

	if (hProcess)
		CloseHandle(hProcess);

	if (pDllPath)
		VirtualFree(pDllPath, 0, MEM_RELEASE);

	return 0; 
}