#include <windows.h>

#define WM_MY_CUSTOM_MSG (WM_USER + 1)

LPRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MY_CUSTOM_MSG:
		// 这里处理你的自定义消息
		MessageBox(0, L"Receive custom message!", L"Notification", MB_OK);
		return 0; 
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

