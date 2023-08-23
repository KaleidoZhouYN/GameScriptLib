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
		// �������ڴ棬����
		Lock lock(mutex);

		// ����header
		SharedDataHeader shm_header;

		// ���ȱ���frame
		memcpy(&shm_header, shm->data<BYTE*>(), sizeof(shm_header));
		int width = shm_header.width, height = shm_header.height, channel = shm_header.channel;
		if (width * height * channel > 0)
		{
			pixels = new BYTE[width * height * channel];

			memcpy(pixels, shm->data<BYTE*>() + sizeof(shm_header), width * height * channel);
		}
		lock.unlock();

		// ת��opencv���Խ��ܵĸ�ʽ
		if (pixels)
		{
			cv::Mat frame(height, width, CV_8UC3, pixels);
			// ���·�ת����ΪopenGL����������½�
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
	// ��ȡĿ�����ID
	const char* keyword = "�׵�";
	WinInfo_t winInfo;
	GetHWndByName(keyword, reinterpret_cast<LPARAM>(&winInfo));
	std::cout << "�ҵ���Ӧ�ĳ��������Ϊ" << std::endl << winInfo.second << std::endl;

	FindHWndRedraw(winInfo.first, reinterpret_cast<LPARAM>(&winInfo));
	std::cout << "�ҵ�VREDRAW�ӳ��������Ϊ" << std::endl << winInfo.second << std::endl; 
	HWND hWnd = winInfo.first;
	return hWnd; 
}

HWND GetYuanshen()
{
	// ��ȡĿ�����ID
	const char* keyword = "ԭ��";
	WinInfo_t winInfo;
	GetHWndByName(keyword, reinterpret_cast<LPARAM>(&winInfo));
	std::cout << "�ҵ���Ӧ�ĳ��������Ϊ" << std::endl << winInfo.second << std::endl;

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
		0, // Ĭ������
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

	// ��Ŀ�����
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
	if (!hProcess) {
		std::cerr << "Error: Could not open process" << std::endl; 
		goto fail; 
	}

	// ��Ŀ������з����ڴ�
	void* pDllPath = VirtualAllocEx(hProcess, 0, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	// дDLL·����Ŀ����̵��ڴ���
	if (!pDllPath) {
		std::cerr << "Error: Could not Alloc Virtual Memory" << std::endl; 
		goto fail;
	}
	
	if (!WriteProcessMemory(hProcess, pDllPath, dllPath, strlen(dllPath) + 1, 0)) {
		std::cout << "Error: Could not write dll path" << std::endl; 
		goto fail; 
	}

	//��Ŀ�����������һ�����߳�������DLL
	HANDLE hThread = CreateRemoteThreadEx(hProcess, 0, 0,
		(LPTHREAD_START_ROUTINE)LoadLibraryA,
		pDllPath, 0, 0, 0);


	// ��ȡbuffer����ʾ

	char t; 
	std::cin >> t; 

	shm = SharedMemorySingleton::Instance(shm_name, MaxShmSize);
	shm->open(); 
	mutex = MutexSingleton::Instance(shm_name+"Mutex");
	mutex->open(); 

	ReadBufferAndShow(); 
	// �����رվ��


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