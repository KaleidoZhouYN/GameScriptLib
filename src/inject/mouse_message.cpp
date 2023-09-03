#include <windows.h>

#include "easylogging++.h"
#define ELPP_THREAD_SAFE

#define DLL_API extern "C" __declspec(dllexport)

HHOOK g_hMouseHook = NULL; 
HWND g_targetHwnd = NULL;
HINSTANCE g_hInstance = NULL;

// 自定义鼠标消息处理函数
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	// 如果nCode小于0，则直接将消息传递给下一个钩子
	if (nCode < 0)
		return CallNextHookEx(NULL, nCode, wParam, lParam);

	// 如果nCode等于HC_ACTION，则处理该消息
	if (nCode == HC_ACTION)
	{
		// 获取鼠标消息的信息
		MOUSEHOOKSTRUCT *pmh = (MOUSEHOOKSTRUCT *)lParam;


		if (wParam == WM_RBUTTONDOWN)
		{
			LOG(INFO) << "R BUTTON DOWN";
			PostMessage(g_targetHwnd, WM_USER + 1, 0, 0);
		}
	}

	// 将该消息传递给下一个钩子
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

struct Param
{
	DWORD pid;
	HWND hwnd;
};

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	DWORD pid;
	Param* p = (Param*)lParam;

	if (!GetWindowThreadProcessId(hwnd, &pid))
		return TRUE; // 继续枚举

	if (pid == p->pid)
	{
		// 可以通过各种方法增加筛选条件，如检查窗口是否可见等
		if (IsWindowVisible(hwnd))
		{
			LOG(INFO) << "Found window: " << pid;
			p->hwnd = hwnd; // 找到匹配的窗口句柄
			return FALSE;   // 停止枚举
		}
	}

	return TRUE; // 继续枚举
}

HWND GetMainWindowHandleFromProcessID(DWORD pid)
{
	Param p;
	p.pid = pid;
	p.hwnd = NULL;

	EnumWindows(EnumWindowsProc, (LPARAM)&p);

	return p.hwnd;
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

	LOG(ERROR) << L"Error (" << error << "): " << (LPSTR)errMsg;

	LocalFree(errMsg);
}

DLL_API long SetHook(DWORD hProcessId)
{
	// 安装鼠标钩子
	// 由hProcessId 获取 hwnd
	g_targetHwnd = GetMainWindowHandleFromProcessID(hProcessId);
	g_hMouseHook = SetWindowsHookEx(WH_MOUSE, MouseProc, g_hInstance, 0);
	if (!g_hMouseHook) {
		LOG(ERROR) << "SetWindowsHookEx failed.";
		PrintLastError();
	}
	return 0; 
}

DLL_API long ReleaseHook()
{
	if (g_hMouseHook)
	{
		UnhookWindowsHookEx(g_hMouseHook); 
		g_hMouseHook = NULL; 
	}
	return 0; 
}

INITIALIZE_EASYLOGGINGPP
BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	el::Configurations conf;
	conf.setToDefault();
	conf.set(el::Level::Global, el::ConfigurationType::Filename, R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\tests\app\epic7\mouse.log)");  // 设置日志文件的路径
	el::Loggers::reconfigureLogger("default", conf); // 重新配置默认的 logger
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hInstance = hModule;
		break; 
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}