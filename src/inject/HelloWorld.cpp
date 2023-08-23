#include <Windows.h>

// ÊµÏÖHelloWorldº¯Êý

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		MessageBoxA(0, "DLL process has been injected!", "Injected", MB_ICONEXCLAMATION);
		break; 
	case DLL_THREAD_ATTACH:
		MessageBoxA(0, "DLL thread has been injected!", "Injected", MB_ICONEXCLAMATION);
		break;
	case DLL_PROCESS_DETACH:
	case DLL_THREAD_DETACH:
		break; 
	}
	return TRUE;
}