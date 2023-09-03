#include <windows.h>

#include "easylogging++.h"
#define ELPP_THREAD_SAFE

#define DLL_API extern "C" __declspec(dllexport)

HHOOK g_hMouseHook = NULL; 
HWND g_targetHwnd = NULL;
HINSTANCE g_hInstance = NULL;

// �Զ��������Ϣ������
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	// ���nCodeС��0����ֱ�ӽ���Ϣ���ݸ���һ������
	if (nCode < 0)
		return CallNextHookEx(NULL, nCode, wParam, lParam);

	// ���nCode����HC_ACTION���������Ϣ
	if (nCode == HC_ACTION)
	{
		// ��ȡ�����Ϣ����Ϣ
		MOUSEHOOKSTRUCT *pmh = (MOUSEHOOKSTRUCT *)lParam;


		if (wParam == WM_RBUTTONDOWN)
		{
			LOG(INFO) << "R BUTTON DOWN";
			PostMessage(g_targetHwnd, WM_USER + 1, 0, 0);
		}
	}

	// ������Ϣ���ݸ���һ������
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
		return TRUE; // ����ö��

	if (pid == p->pid)
	{
		// ����ͨ�����ַ�������ɸѡ���������鴰���Ƿ�ɼ���
		if (IsWindowVisible(hwnd))
		{
			LOG(INFO) << "Found window: " << pid;
			p->hwnd = hwnd; // �ҵ�ƥ��Ĵ��ھ��
			return FALSE;   // ֹͣö��
		}
	}

	return TRUE; // ����ö��
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
		0, // Ĭ������
		(LPSTR)&errMsg,
		0,
		NULL
	);

	LOG(ERROR) << L"Error (" << error << "): " << (LPSTR)errMsg;

	LocalFree(errMsg);
}

DLL_API long SetHook(DWORD hProcessId)
{
	// ��װ��깳��
	// ��hProcessId ��ȡ hwnd
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
	conf.set(el::Level::Global, el::ConfigurationType::Filename, R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\tests\app\epic7\mouse.log)");  // ������־�ļ���·��
	el::Loggers::reconfigureLogger("default", conf); // ��������Ĭ�ϵ� logger
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