#include <windows.h>

#define DLL_API extern "C" __declspec(dllexport)

HHOOK g_hMouseHook = NULL; 
HWND g_targetHwnd = NULL;

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

DLL_API long __stdcall SetHook(DWORD hProcessId)
{
	// 安装鼠标钩子
	// 由hProcessId 获取 hwnd
	g_targetHwnd = GetMainWindowHandleFromProcessID(hProcessId);
	g_hMouseHook = SetWindowsHookEx(WH_MOUSE, MouseProc, GetModuleHandle(NULL), 0);
	return 0; 
}

DLL_API long __stdcall ReleaseHook()
{
	if (g_hMouseHook)
	{
		UnhookWindowsHookEx(g_hMouseHook); 
		g_hMouseHook = NULL; 
	}
	return 0; 
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}