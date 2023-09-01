#include <windows.h>
#include "win.h"
#include "injector.h"

#define ID_START_BTN 101
#define ID_AUTOSTART_BTN 102
#define ID_INPUT_BTN 103
#define ID_END_BTN 104

#define ID_COMBOBOX 201

#define WM_USER_CUSTOM WM_USER + 1

INITIALIZE_EASYLOGGINGPP

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HWND hwnd;
int WINAPI wWinMain(_In_ HINSTANCE hInstance,
_In_opt_ HINSTANCE hPrevInstance,
_In_ LPWSTR    lpCmdLine,
_In_ int       nCmdShow) {
    el::Configurations conf;
    conf.setToDefault();
    conf.set(el::Level::Global, el::ConfigurationType::Filename, R"(test_all.log)");  // 设置日志文件的路径
    el::Loggers::reconfigureLogger("default", conf); // 重新配置默认的 logger

    static TCHAR szAppName[] = TEXT("E7DemoApp");
    MSG msg;
    WNDCLASS wndclass;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass(&wndclass)) {
        MessageBox(NULL, TEXT("Failed to register window class!"), szAppName, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindow(szAppName, TEXT("Demo App"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        400, 300,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

HWND g_hwndComboBox;
WinTitleList_t g_windowTitle_List;
HWND g_target_hwnd = NULL;
Injector* g_mouse_injector = NULL; 

void reset_resource()
{
    if (g_mouse_injector)
    {
        g_mouse_injector->release_hook("ReleaseHook", NULL);
        g_mouse_injector->release(); 
        delete g_mouse_injector;
        g_mouse_injector = NULL; 
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        CreateWindow(TEXT("button"), TEXT("开始"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            20, 20, 100, 30, hwnd, (HMENU)ID_START_BTN, NULL, NULL);

        CreateWindow(TEXT("button"), TEXT("自动开始"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            130, 20, 100, 30, hwnd, (HMENU)ID_AUTOSTART_BTN, NULL, NULL);

        CreateWindow(TEXT("button"), TEXT("录入"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            20, 70, 100, 30, hwnd, (HMENU)ID_INPUT_BTN, NULL, NULL);

        CreateWindow(TEXT("button"), TEXT("结束"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            130, 70, 100, 30, hwnd, (HMENU)ID_END_BTN, NULL, NULL);

        g_hwndComboBox = CreateWindow(
            TEXT("COMBOBOX"),
            TEXT(""),
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWN,
            20, 120, 150, 300, // x, y, width, height
            hwnd,
            (HMENU)ID_COMBOBOX,
            NULL,
            NULL);
    }
                  return 0;

    case WM_USER_CUSTOM:
        MessageBox(hwnd, TEXT("Received WM_USER_CUSTOM message!"), TEXT("Notification"), MB_OK);
        return 0;

    case WM_COMMAND:
        // Handle button clicks here if needed.
        switch (LOWORD(wParam)) {
        case ID_START_BTN:
            if (g_target_hwnd == NULL) {
                MessageBox(hwnd, "请选择窗口", "提示", MB_OK);
                break;
            }
            // 开始注入
            if (!g_mouse_injector)
            {
                // 获取控制窗口
                //WinTitleList_t childList;
                WinInfo_t winInfo; 
                FindHWndRedraw(g_target_hwnd, reinterpret_cast<LPARAM>(&winInfo));

                //GetHWndChild(g_target_hwnd, reinterpret_cast<LPARAM>(&childList));
                //auto child = childList[0];
                //g_mouse_injector = new Injector(child.first);
                g_mouse_injector = new Injector(winInfo.first);
                const char* dllpath = R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\src\inject\MouseDLL.dll)";
                g_mouse_injector->inject(dllpath);
                DWORD pid; 
                GetWindowThreadProcessId(hwnd, &pid);
                g_mouse_injector->set_message_hook(pid);
            }
        case ID_AUTOSTART_BTN:
            // 假设已经注入成功
            
            break;
        case ID_END_BTN:
            break; 
        case ID_INPUT_BTN:
            break; 
        }
        if (HIWORD(wParam) == CBN_DROPDOWN && LOWORD(wParam) == ID_COMBOBOX) {
            // 用户打开了ID_COMBOBOX的下拉列表
            // 清空下拉列表
            SendMessage(g_hwndComboBox, CB_RESETCONTENT, 0, 0);
            // 更新窗口列表
            g_windowTitle_List.clear();
            EnumWindows(&EnumWindowsTitleProc, reinterpret_cast<LPARAM>(&g_windowTitle_List));

            for (auto it : g_windowTitle_List)
            {
                SendMessage(g_hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT(it.second.c_str())); 
            }
        }

        // 用户选取了下拉列表中的某个项
        if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == ID_COMBOBOX) {
			// 获取当前选中项的索引
            reset_resource(); 
			int index = SendMessage(g_hwndComboBox, CB_GETCURSEL, 0, 0);
			g_target_hwnd = g_windowTitle_List[index].first;
            MessageBox(hwnd, g_windowTitle_List[index].second.c_str(), "ok", MB_OK);
		}
        return 0;

    case WM_DESTROY:
        reset_resource(); 
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}