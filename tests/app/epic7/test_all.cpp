#include <windows.h>
#include "win.h"
#include "capture_gl.h"
#include "easylogging++.h"
#include <opencv2/opencv.hpp>
#include "onnx_infer.h"
#include "controller.h"
#include <atomic>
#include <chrono>

#define ID_START_BTN 101
#define ID_AUTOSTART_BTN 102
#define ID_INPUT_BTN 103
#define ID_END_BTN 104

#define ID_COMBOBOX 201

#define WM_USER_CUSTOM WM_USER + 1

INITIALIZE_EASYLOGGINGPP

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void reset_resource();
HHOOK hMouseHook;
HWND g_control_hwnd = NULL;
std::thread global_t2; 

HWND g_hwndComboBox;
WinTitleList_t g_windowTitle_List;
HWND g_target_hwnd = NULL;
std::shared_ptr<OpenglCapture> g_capture;
std::shared_ptr<Controller> g_controller;
std::shared_ptr<OnnxInfer> g_onnx_infer;
std::atomic<bool> should_run(true);

std::thread global_t1;
cv::Mat g_image; 

const int g_class_num = 214;


int get_model_result(const cv::Mat& image)
{
    cv::Rect rect(1140, 272, 140, 90);
    cv::Mat croppedImage = image(rect);
    //cv::resize(croppedImage, croppedImage, cv::Size(64, 48));
    cv::imwrite("temp.jpg", image);
    croppedImage.convertTo(croppedImage, CV_32FC3);
    croppedImage /= 255.0f;

    // [H,W,C] -> [C,H,W]
    std::vector<cv::Mat> channels(3);
    cv::split(croppedImage, channels);

    cv::Mat chw(croppedImage.rows * croppedImage.cols * 3, 1, CV_32F);
    for (int i = 0; i < 3; ++i) {
        channels[i].reshape(1, croppedImage.rows * croppedImage.cols).copyTo(chw.rowRange(i * croppedImage.rows * croppedImage.cols, (i + 1) * croppedImage.rows * croppedImage.cols));
    }

    float* p = chw.ptr<float>(0);

    std::vector<Ort::Value> inputs;
    inputs.emplace_back(pt_to_tensor<float>(p, g_onnx_infer->get_input_shapes()[0]));
    //onnx_infer.set_inputs(inputs);

    g_onnx_infer->forward(inputs);
    auto* results_tensors = g_onnx_infer->get_result("84");

    float max = -1000;
    int max_index = -1;

    float* results_ = const_cast<Ort::Value*>(results_tensors)->GetTensorMutableData<float>();
    for (int i = 0; i < g_class_num; i++)
        if (max < results_[i])
        {
            max = results_[i];
            max_index = i;
        }
    return max_index;
}

void click_thread()
{
    FrameInfo frame;
    g_capture->capture_frame(&frame);
    if (frame.usable) {
        cv::Mat image = cv::Mat(frame.header.height, frame.header.width, CV_8UC3, frame.buffer);
        cv::flip(image, image, 0);
        cv::resize(image, image, cv::Size(1280, 800));
        int max_index = get_model_result(image);
        std::string path = "C:\\Users\\zhouy\\Desktop\\test\\";
        std::string name = std::to_string(max_index) + ".jpg";
        if (max_index < 0)
            name = "unknown.jpg";

        g_image = cv::imread(path + name);
    }
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        MOUSEHOOKSTRUCT* ms = (MOUSEHOOKSTRUCT*)lParam;

        HWND hwndFromPoint = WindowFromPoint(ms->pt);

        if (hwndFromPoint == g_control_hwnd)
        {
            // 这个鼠标消息是发给我们的目标窗口的
            switch (wParam)
            {
            case WM_RBUTTONDOWN:
            {
                cv::namedWindow("hero");
                global_t2 = std::thread(click_thread);
                global_t2.join(); 
                cv::imshow("hero", g_image);
                cv::waitKey(25);
            }
                break;
                // ... 其他鼠标消息处理 ...
            }
        }
    }

    return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

HWND hwnd;
HINSTANCE g_hInstance;
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
    g_hInstance =  hInstance;
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

    try {
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    catch (...) {
        reset_resource();
    }

    return msg.wParam;
}

void move_to_next()
{
    float TargetX_p = 1714.00 / 1920.0;
    float TargetY_p = 530.0 / 1080;
    POINT HW = g_controller->get_window_HW();
    int TargetX = static_cast<int>(TargetX_p * HW.x);
    int TargetY = static_cast<int>(TargetY_p * HW.y);
    g_controller->mouse_lclick(TargetX, TargetY, 0.1);
}

void move_to_pred()
{
    float TargetX_p = 1714.00 / 1920.0;
    float TargetY_p = 330.0 / 1080;
    POINT HW = g_controller->get_window_HW();
    int TargetX = static_cast<int>(TargetX_p * HW.x);
    int TargetY = static_cast<int>(TargetY_p * HW.y);
    g_controller->mouse_lclick(TargetX, TargetY, 0.1);
}


void auto_thread()
{
    move_to_next(); 
    Sleep(1 * 1000);
    move_to_pred();
    Sleep(1 * 1000);
    for (int i = 0; i < 300; i++)
    {
        FrameInfo frame;
        g_capture->capture_frame(&frame);
        if (frame.usable) {
            cv::Mat image = cv::Mat(frame.header.height, frame.header.width, CV_8UC3, frame.buffer);
            cv::flip(image, image, 0);
            cv::resize(image, image, cv::Size(1280, 800));
            cv::Mat ori_image = image.clone();
            int max_index = get_model_result(image);
            std::string path = "C:\\Users\\zhouy\\Desktop\\test\\";
            std::string name = std::to_string(max_index) + ".jpg";
            if (max_index < 0)
                name = "unknown.jpg";
            cv::imwrite(path + name, ori_image);

        }     
        move_to_next();
        Sleep(1 * 1000);
        if (!should_run)
            break; 
    }
}

void reset_resource()
{
    g_capture.reset(); 
    g_controller.reset(); 
    g_onnx_infer.reset(); 
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
        case ID_START_BTN: {
            if (g_target_hwnd == NULL) {
                MessageBox(hwnd, "请选择窗口", "提示", MB_OK);
                break;
            }
            // 开始注入
            WinInfo_t winInfo;
            FindHWndRedraw(g_target_hwnd, reinterpret_cast<LPARAM>(&winInfo));
            const char* dllPath = R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\src\inject\OpenGLScreenShotDLL.dll)";
            const size_t MaxShmSize = 2560 * 1600 * 3;
            WinTitleList_t childList;
            GetHWndChild(g_target_hwnd, reinterpret_cast<LPARAM>(&childList));
            auto child = childList[0];
            g_control_hwnd = child.first;
            g_capture = std::make_shared<OpenglCapture>(winInfo.first, dllPath, MaxShmSize);
            g_capture->start();
            g_onnx_infer = std::make_shared<OnnxInfer>();
            g_onnx_infer->load(R"(C:\Users\zhouy\source\repos\GameScriptLib\src\app\epic7\assert\hero_avatar.onnx)");
            std::vector<std::string> output_name_ = { "84" };
            std::vector<std::vector<int64_t>> output_shape_ = { {1, g_class_num} };
            g_onnx_infer->set_outputs(output_name_, output_shape_);

            hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, g_hInstance, 0);
        }
                         break; 
        case ID_AUTOSTART_BTN: {
            // 假设已经注入成功
            g_controller = std::make_shared<Controller>(g_control_hwnd);
            // 加载模型
            // 开启线程
            should_run = true; 
            global_t1 = std::thread(auto_thread);
        }
            break;
        case ID_END_BTN: {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            should_run = false;
        }
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