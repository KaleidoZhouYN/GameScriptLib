#include <windows.h>
#include <atomic>
#include <chrono>
#include <shellapi.h>
#include <ctime>
#include<iostream>
#include<filesystem>
#include <opencv2/opencv.hpp>

#include "win.h"
#include "capture_gl.h"
#include "onnx_infer.h"
#include "controller.h"

#include "easylogging++.h"
#define ELPP_THREAD_SAFE

// macro
INITIALIZE_EASYLOGGINGPP

// extern functions and variables
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void reset_resource();

// global variables
HWND hwnd;
HINSTANCE g_hInstance;
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
std::string g_path; 
std::atomic<bool> g_show(false);
NOTIFYICONDATA nid = { sizeof(nid) };

size_t g_hero_cnt = 0; 
HMENU hMenu;


// constant 
const UINT ID_START_BTN = 101;
const UINT ID_AUTOSTART = 102;
const UINT ID_AUTOSTOP = 103; 
const UINT ID_INPUT = 104;
const UINT ID_SHOW = 105;
const UINT ID_SAVE_RTA = 106; 
const UINT ID_END = 107;
const UINT ID_CLOSE_MENU = 108;
const UINT ID_COMBOBOX = 201;
const UINT WM_TRAYICON = WM_USER + 1;


const int g_class_num = 214;

std::string g_save_path = std::filesystem::current_path().string() + "\\";
std::string g_img_save_path = g_save_path + "images\\";
std::string g_fea_save_path = g_save_path + "features\\";
std::string g_rta_save_path = g_save_path + "rta\\";
std::string g_capture_dllPath = g_save_path + R"(OpenGLScreenShotDLL.dll)";
std::string g_model_path = g_save_path + R"(assert\\hero_avatar_feature.onnx)";
std::vector<std::string> g_output_name_ = { "64" };
constexpr size_t fea_dim = 32;
std::vector<std::vector<int64_t>> g_output_shape_ = { {1, fea_dim} };

std::vector<std::array<float, fea_dim>> hero_fea(0);
const float g_sim_threshold = 0.8; 

void read_hero_fea()
{
    int fileCount = 0;
    for (const auto& entry : std::filesystem::directory_iterator(g_fea_save_path)) {
        if (std::filesystem::is_regular_file(entry.path())) {
            fileCount++;
        }
    }
    
    for (int i = 0; i < fileCount; i++)
    {
        std::string filepath = g_fea_save_path + std::to_string(i) + ".bin";
        std::ifstream in(filepath);
        std::array<float, fea_dim> arr;
        for (float& num : arr)
        {
            in >> num; 
        }
        hero_fea.push_back(arr); 
        g_hero_cnt += 1; 
    }
}

float get_similarity(const std::array<float, fea_dim>& f1, const std::array<float, fea_dim>& f2)
{
    float result = 0; 
    for (int i = 0; i < fea_dim; i++)
    {
        result += f1[i] * f2[i];
    }
    return result; 
}

int get_max_match(const std::array<float, fea_dim>& f)
{
    float max_value = -1; 
    int max_index = -1; 
    for (int i = 0; i < hero_fea.size(); i++)
    {
        float sim = get_similarity(f, hero_fea[i]);
        if (max_value < sim)
        {
            max_value = sim; 
            max_index = i; 
        }
    }

    if (max_value < g_sim_threshold)
        max_index = -1; 

    return max_index; 
}

//============================================

std::array<float,fea_dim> get_model_result(const cv::Mat& image)
{
    cv::Rect rect(1140, 272, 140, 90);
    cv::Mat croppedImage = image(rect);
    //cv::resize(croppedImage, croppedImage, cv::Size(64, 48));
    //cv::imwrite("temp.jpg", image);
    croppedImage.convertTo(croppedImage, CV_32FC3);
    croppedImage /= 255.0f;

    // [H,W,C] -> [C,H,W]
    std::vector<cv::Mat> channels(3);
    cv::split(croppedImage, channels);

    cv::Mat chw(croppedImage.rows * croppedImage.cols * 3, 1, CV_32F);
    for (int i = 0; i < 3; ++i) {
        channels[i].reshape(1, croppedImage.rows * croppedImage.cols).copyTo(
            chw.rowRange(i * croppedImage.rows * croppedImage.cols, (i + 1) * croppedImage.rows * croppedImage.cols));
    }

    float* p = chw.ptr<float>(0);

    std::vector<Ort::Value> inputs;
    inputs.emplace_back(pt_to_tensor<float>(p, g_onnx_infer->get_input_shapes()[0]));

    g_onnx_infer->forward(inputs);
    auto* results_tensors = g_onnx_infer->get_result(g_output_name_[0]);

    float* results_ = const_cast<Ort::Value*>(results_tensors)->GetTensorMutableData<float>();

    /*
    for (int i = 0; i < g_class_num; i++)
        if (max < results_[i])
        {
            max = results_[i];
            max_index = i;
        }
    return max_index;
    */
    std::array<float, fea_dim> arr; 
    std::copy(results_, results_ + fea_dim, arr.begin());
    return arr;
}


LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        MOUSEHOOKSTRUCT* ms = (MOUSEHOOKSTRUCT*)lParam;

        // 如果目标窗口是活动的，且鼠标位置在目标窗口内，则处理消息
        if (g_target_hwnd == NULL)
            goto end; 
        RECT rect;
        ::GetWindowRect(g_target_hwnd, &rect);

        UINT dpi = GetDpiForWindow(g_target_hwnd);
        float scale = static_cast<float>(dpi) / STD_DPI;
        POINT pt = ms->pt; 
        pt.x = static_cast<int>(pt.x / scale);
        pt.y = static_cast<int>(pt.y / scale);

        BOOL isInRect = false; 
        if (pt.x >= rect.left && pt.x <= rect.right && pt.y >= rect.top && pt.y <= rect.bottom)
            isInRect = true; 

        if (::GetForegroundWindow() == g_target_hwnd && isInRect)
        {
            switch (wParam)
            {
            case WM_RBUTTONUP:
            {
                if (hMenu)
                {
                    DestroyMenu(hMenu);
                }
                // 创建一个菜单
                hMenu = CreatePopupMenu(); 
                if (!hMenu)
                    return 0; 

                AppendMenuA(hMenu, MF_STRING, ID_CLOSE_MENU, "关闭菜单");
                AppendMenuA(hMenu, MF_STRING, ID_AUTOSTART, "自动开始录入");
                AppendMenuA(hMenu, MF_STRING, ID_AUTOSTOP, "停止自动录入");
                AppendMenuA(hMenu, MF_STRING, ID_INPUT, "录入当前装备");
                AppendMenuA(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenuA(hMenu, MF_STRING, ID_SHOW, "显示装备");
                AppendMenuA(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenuA(hMenu, MF_STRING, ID_SAVE_RTA, "截取RTA记录");
                AppendMenuA(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenuA(hMenu, MF_STRING, ID_END, "结束程序");

                TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
            }
                break;
                // ... 其他鼠标消息处理 ...
            }

        }
    }

end:
    return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance,
_In_opt_ HINSTANCE hPrevInstance,
_In_ LPWSTR    lpCmdLine,
_In_ int       nCmdShow) {

    // 配置日志文件
    el::Configurations conf;
    conf.setToDefault();
    conf.set(el::Level::Global, el::ConfigurationType::Filename, R"(main.log)");  
    el::Loggers::reconfigureLogger("default", conf); 
    LOG(INFO) << "\nSTART OF AN NEW APP+++++++++++++++++++++++";

    // 读取数据库
    std::filesystem::create_directory(g_save_path);
    std::filesystem::create_directory(g_img_save_path);
    std::filesystem::create_directory(g_fea_save_path);
    std::filesystem::create_directory(g_rta_save_path);
    read_hero_fea(); 
    LOG(INFO) << "Read Hero Features Done";

    //=============

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

    hwnd = CreateWindowA(szAppName, TEXT("Demo App"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        300, 140,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    try {
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (!IsWindow(g_target_hwnd))
            {
                ShowWindow(hwnd, SW_RESTORE);
                Shell_NotifyIcon(NIM_DELETE, &nid);
            }
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
    g_controller->mouse_lclick(TargetX, TargetY, 0.001);
}

void move_to_pred()
{
    float TargetX_p = 1714.00 / 1920.0;
    float TargetY_p = 330.0 / 1080;
    POINT HW = g_controller->get_window_HW();
    int TargetX = static_cast<int>(TargetX_p * HW.x);
    int TargetY = static_cast<int>(TargetY_p * HW.y);
    g_controller->mouse_lclick(TargetX, TargetY, 0.01);
}

void record()
{
    FrameInfo frame;
    g_capture->capture_frame(&frame);
    if (frame.usable) {
        cv::Mat image = cv::Mat(frame.header.height, frame.header.width, CV_8UC3, frame.buffer);
        cv::flip(image, image, 0);
        cv::resize(image, image, cv::Size(1280, 800));
        cv::Mat ori_image;
        cv::resize(image, ori_image, cv::Size(1280, 720));
        std::array<float, fea_dim> arr = get_model_result(image);
        int max_index = get_max_match(arr);
        if (max_index < 0)
        {
            max_index = g_hero_cnt;
            g_hero_cnt += 1; 
            hero_fea.push_back(arr);
        }
        else
        {
            hero_fea[max_index] = arr;
        }
        std::string path = g_img_save_path; 
        std::string name = std::to_string(max_index) + ".jpg";
        cv::imwrite(path + name, ori_image);

        path = g_fea_save_path;
        name = std::to_string(max_index) + ".bin";
        std::ofstream out((path + name));
        for (float num : arr) {
            out << num << " ";
        }
        out.close();
        
        LOG(INFO) << "录入第 " << max_index << " 个英雄";

    }
}

void auto_thread()
{
    //SetWindowPos(g_target_hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    //move_to_next(); 
    //Sleep(1 * 1000);
    //move_to_pred();
    Sleep(1 * 1000);
    for (int i = 0; i < 300; i++)
    {
        record(); 
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

std::string getCurrentDateTime(const std::string& format) {
    std::time_t t = std::time(nullptr);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), format.c_str(), std::localtime(&t));
    return std::string(buffer);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {

        // 主窗口组件
        CreateWindow(TEXT("button"), TEXT("开始"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            100, 60, 100, 30, hwnd, (HMENU)ID_START_BTN, NULL, NULL);


        g_hwndComboBox = CreateWindow(
            TEXT("COMBOBOX"),
            TEXT(""),
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWN,
            20, 20, 260, 300, // x, y, width, height
            hwnd,
            (HMENU)ID_COMBOBOX,
            NULL,
            NULL);


        // 系统托盘设置
        nid.hWnd = hwnd;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_TRAYICON;
        nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        strcpy_s(nid.szTip, sizeof(nid.szTip), "My Application");
    }
        return 0;

    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED) {
            Shell_NotifyIcon(NIM_ADD, &nid);
            ShowWindow(hwnd, SW_HIDE);
        }
        break; 

    case WM_TRAYICON:
        switch (lParam) {
        case WM_LBUTTONDBLCLK:
            ShowWindow(hwnd, SW_RESTORE);
            Shell_NotifyIcon(NIM_DELETE, &nid);
            break;
        }
        break;

    case WM_COMMAND:
        // Handle button clicks here if needed.
        switch (LOWORD(wParam)) {
        case ID_START_BTN: {
            if (g_target_hwnd == NULL) {
                MessageBoxA(hwnd, "请选择窗口", "提示", MB_OK);
            }
            else
            {
                // 先重置所有资源
                reset_resource();

                // 获取渲染窗口 && 注入capture
                WinInfo_t winInfo;
                FindHWndRedraw(g_target_hwnd, reinterpret_cast<LPARAM>(&winInfo));
                ;
                const size_t MaxShmSize = 2560 * 1600 * 3;
                g_capture = std::make_shared<OpenglCapture>(winInfo.first, g_capture_dllPath, MaxShmSize);
                g_capture->start();

                // 获取控制窗口
                WinTitleList_t childList;
                GetHWndChild(g_target_hwnd, reinterpret_cast<LPARAM>(&childList));
                auto child = childList[0];
                g_control_hwnd = child.first;

                // 启动识别模型
                g_onnx_infer = std::make_shared<OnnxInfer>();
                g_onnx_infer->load(g_model_path.c_str());
                g_onnx_infer->set_outputs(g_output_name_, g_output_shape_);

                g_controller = std::make_shared<Controller>(g_control_hwnd);

                // 开始捕获全局鼠标消息
                hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, g_hInstance, 0);

                MessageBoxA(hwnd, "关联成功", "提示", MB_OK);

                // 隐藏窗口到任务栏
                Shell_NotifyIcon(NIM_ADD, &nid);
                ShowWindow(hwnd, SW_HIDE);

                LOG(INFO) << "关联成功";
            }

            break;
        }
        case ID_AUTOSTART: {
            // 假设已经注入成功
            LOG(INFO) << "开始自动录入";
            // 开启线程
            should_run = true;
            global_t1 = std::thread(auto_thread);
            break;
        }
        case ID_AUTOSTOP: {
            // 结束自动录入
            LOG(INFO) << "结束自动录入";
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            should_run = false;
            break;
        }
        case ID_INPUT:
            // 录入当前的装备页面
            record();
            break;

        case ID_SHOW:
        {
            // 显示装备
            FrameInfo frame;
            g_capture->capture_frame(&frame);
            if (frame.usable) {
                cv::Mat image = cv::Mat(frame.header.height, frame.header.width, CV_8UC3, frame.buffer);
                cv::flip(image, image, 0);
                cv::resize(image, image, cv::Size(1280, 800));
                int max_index = get_max_match(get_model_result(image));
                if (max_index < 0)
                {
                    MessageBoxA(0, "该英雄尚未录入", "提示", MB_OK);
                    g_show = false;
                    break; 
                }
                std::string path = g_img_save_path;
                std::string name = std::to_string(max_index) + ".jpg";
                g_show = true;

                image = cv::imread(path + name);
                
                cv::Rect rect1(20, 75, 450-20, 700-75);
                cv::Mat img1 = image(rect1).clone();
                cv::Rect rect2(835, 75, 990-835, 700-75);
                cv::Mat img2 = image(rect2).clone();
                cv::hconcat(img1, img2, image);
                

                cv::namedWindow("hero");
                cv::imshow("hero", image);
                cv::waitKey(25);
                RECT rect;
                ::GetWindowRect(g_target_hwnd, &rect);
                //cv::moveWindow("hero", rect.left + (rect.right-rect.left)/3.5, rect.top);
            }
            break;
        }
        case ID_END: {
            reset_resource();
            ShowWindow(hwnd, SW_RESTORE);
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            return 0;
        }

        case ID_SAVE_RTA: {
            // 打马赛克并保存图片
            FrameInfo frame;
            g_capture->capture_frame(&frame);
            if (frame.usable) {
                cv::Mat image = cv::Mat(frame.header.height, frame.header.width, CV_8UC3, frame.buffer);
                cv::flip(image, image, 0);
                cv::resize(image, image, cv::Size(1280, 720));
                cv::Rect rect1(240, 110, 800, 70);
                cv::Rect rect2(208, 30, 865, 640);
                cv::Mat subImage = image(rect1);
                subImage.setTo(cv::Scalar(0, 0, 0));
                subImage = image(rect2);
                std::string dateTimeStr = getCurrentDateTime("%Y-%m-%d_%H-%M-%S");
                std::string outputPath = g_rta_save_path + dateTimeStr + ".jpg";
                cv::imwrite(outputPath, subImage);
            }
            break;
        }
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
			int index = SendMessage(g_hwndComboBox, CB_GETCURSEL, 0, 0);
			g_target_hwnd = g_windowTitle_List[index].first;
            LOG(INFO) << "选择关联 ：" << g_windowTitle_List[index].second;
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