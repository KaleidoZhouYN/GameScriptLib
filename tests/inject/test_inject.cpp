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
 
const std::string shm_name = "CaptureBuffer_";
const int MaxShmSize = 2560 * 1600 * 3;


// 2023/08/25
// to do : ����һ��֡��ѡȡ������֡�ʹ���
BOOL ReadBufferAndShow(ScreenShotHook* sh, const std::string& name_)
{
	cv::namedWindow(name_, cv::WINDOW_AUTOSIZE);
	BYTE* pixels = nullptr;
	while (TRUE) {
		// �������ڴ棬����
		Lock lock(sh->mutex);

		// ����header
		SharedDataHeader shm_header;

		// ���ȱ���frame
		memcpy(&shm_header, sh->shm->data<BYTE*>(), sizeof(shm_header));
		int width = shm_header.width, height = shm_header.height, channel = shm_header.channel;
		if (width * height * channel > 0)
		{
			pixels = new BYTE[width * height * channel];

			memcpy(pixels, sh->shm->data<BYTE*>() + sizeof(shm_header), width * height * channel);
		}
		lock.unlock();
		

		// ת��opencv���Խ��ܵĸ�ʽ
		std::unique_lock<std::mutex> dis_lock(mtx);
		if (pixels)
		{
			cv::Mat frame(height, width, CV_8UC3, pixels);
			// ���·�ת����ΪopenGL����������½�
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
	// ��ȡĿ�����ID
	WinInfo_t winInfo;
	GetHWndByName(keyword.c_str(), reinterpret_cast<LPARAM>(&winInfo));
	std::cout << "�ҵ���Ӧ�ĳ��������Ϊ" << std::endl << winInfo.second << std::endl;

	FindHWndRedraw(winInfo.first, reinterpret_cast<LPARAM>(&winInfo));
	std::cout << "�ҵ�VREDRAW�ӳ��������Ϊ" << std::endl << winInfo.second << std::endl; 
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

	// ��ȡbuffer����ʾ

	std::stringstream ss;
	ss << shm_name << processId;
	std::shared_ptr<ScreenShotHook> sh(new ScreenShotHook(ss.str(), MaxShmSize));
	sh->start();

	ReadBufferAndShow(sh.get(), ss.str());
	// �����رվ��

	injector.release();
}

void signalHandler(int signum) {

	exit(signum);  // ��ֹ����
}

INITIALIZE_EASYLOGGINGPP

int main() {
	signal(SIGINT, signalHandler); 

	el::Configurations conf;
	conf.setToDefault();
	conf.set(el::Level::Global, el::ConfigurationType::Filename, R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\tests\inject\test_inject.log)");  // ������־�ļ���·��
	el::Loggers::reconfigureLogger("default", conf); // ��������Ĭ�ϵ� logger

	std::thread t1(threadFunction, "�׵�ģ����");
	std::thread t2(threadFunction,"�׵�ģ����-1");
	t1.join();
	t2.join(); 
	return 0; 
}