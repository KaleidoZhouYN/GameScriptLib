#include <windows.h>

#define DLL_API extern "C" __declspec(dllexport)

HHOOK g_hMouseHook = NULL; 
HWND g_targetHwnd = NULL;

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

DLL_API long __stdcall SetHook(DWORD hProcessId)
{
	// ��װ��깳��
	// ��hProcessId ��ȡ hwnd
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