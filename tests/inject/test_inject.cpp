#include "win.h"
#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"
#include <signal.h>
#include <capture_gl.h>

// multi-thread mutex
std::mutex mtx; 


// 2023/08/25
// to do : ����һ��֡��ѡȡ������֡�ʹ���
BOOL ReadBufferAndShow(OpenglCapture* sh, const std::string& index)
{
	std::string name_ = sh->get_name() + std::string("_") + index;
	cv::namedWindow(name_, cv::WINDOW_AUTOSIZE);
	
	while (TRUE) {
		FrameInfo pixels;
		sh->capture_frame(&pixels);
		// ת��opencv���Խ��ܵĸ�ʽ
		std::unique_lock<std::mutex> dis_lock(mtx);
		if (pixels.usable)
		{
			cv::Mat frame(pixels.header.height, pixels.header.width, CV_8UC3, pixels.buffer);
			// ���·�ת����ΪopenGL����������½�
			cv::flip(frame, frame, 0);
			cv::resize(frame, frame, cv::Size(400, 225));
			//cv::imwrite(R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\tests\inject\screenshot_SM.bmp)", frame);
			cv::imshow(name_, frame);
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


void threadFunction(OpenglCapture* capture, const std::string& index) {

	// injector

	// ��ȡbuffer����ʾ
	ReadBufferAndShow(capture, index);
	// �����رվ��
}

std::shared_ptr<OpenglCapture> capture;

void signalHandler(int signum) {
	capture->end(); 
	exit(signum);  // ��ֹ����
}

INITIALIZE_EASYLOGGINGPP

int main() {
	signal(SIGINT, signalHandler); 

	el::Configurations conf;
	conf.setToDefault();
	conf.set(el::Level::Global, el::ConfigurationType::Filename, R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\tests\inject\test_inject.log)");  // ������־�ļ���·��
	el::Loggers::reconfigureLogger("default", conf); // ��������Ĭ�ϵ� logger

	HWND hwnd = GetHWnd("�׵�ģ����");


	const char* dllPath = R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\src\inject\OpenGLScreenShotDLL.dll)";

	const size_t MaxShmSize = 2560 * 1600 * 3;
	capture = std::make_shared<OpenglCapture>(hwnd, dllPath, MaxShmSize);
	capture->start();

	std::thread t1(threadFunction, capture.get(), "1");
	std::thread t2(threadFunction, capture.get(), "2");
	t1.join();
	t2.join(); 
	return 0; 
}