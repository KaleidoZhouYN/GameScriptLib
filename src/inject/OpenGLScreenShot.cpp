#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include "detours.h"
#include "frame_info.h"
#include "ipcrw.h"
#include <sstream>
#include <functional>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "easylogging++.h"
#define ELPP_THREAD_SAFE


// global variable
typedef BOOL(WINAPI* SwapBuffersType)(HDC hdc);
SwapBuffersType orig_SwapBuffers = NULL; 
//std::shared_ptr<ScreenShotHook> global_sh; 
std::shared_ptr<IPCRW> g_ipcrw; 


#define DLL_API extern "C" __declspec(dllexport)


// To do : Change width, heigth to RParam
BOOL GetWinPixels(HDC hdc, LPARAM lParam, int& width, int& height) {
    HWND hwnd = WindowFromDC(hdc);
    RECT rect;
    GetClientRect(hwnd, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    BYTE** pixels = reinterpret_cast<BYTE**>(lParam);
    *pixels = new BYTE[width * height * 3];

    // 使用glReadPixels获取当前帧的像素数据
    glReadBuffer(GL_BACK);  // 设置为读取后缓冲区
    glReadPixels(0, 0, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, *pixels);  // 注意: 使用GL_BGR_EXT因为BMP的数据布局,BMP为BGR,openGL标准格式为RGB
    return TRUE; 
}

DLL_API long __stdcall ReleaseHook();
/*
@brief: swapbuffer的钩子函数, 将frame binary data 保存进入shared memory
@param: hdc, 窗口的Device Context Handle
*/

std::mutex mtx; 
std::condition_variable cv; 
FrameInfo shared_frame; 
bool ready = false;  // 避免虚假唤醒
std::thread cus_thread; 
std::atomic<bool> terminate_flag(false);

void WINAPI customer()
{
    while (!terminate_flag)
    {
        std::unique_lock<std::mutex> lock(mtx);
        while (!ready)
        {
            cv.wait(lock);
        }
        auto boundFn = std::bind(&FrameInfo::write, &shared_frame, std::placeholders::_1);
        g_ipcrw->write_to_sm(boundFn);
        shared_frame.delete_buffer();
        ready = false; 
    }
}

BOOL WINAPI hooked_SwapBuffers_SharedMemory(HDC hdc)
{
    try {
        if (mtx.try_lock())
        {
            //FrameInfo frame;
            int height = 0, width = 0;
            GetWinPixels(hdc, reinterpret_cast<LPARAM>(&shared_frame.buffer), width, height);
            SharedDataHeader shm_header = { width, height, 3 };

            shared_frame.header = shm_header;
            ready = true;
            cv.notify_all();
            mtx.unlock(); 
        }
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "捕获到异常:" << e.what();
        ReleaseHook();
    }
    return orig_SwapBuffers(hdc);
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

    MessageBoxA(0, (LPSTR)errMsg, "fail", MB_ICONEXCLAMATION);

    LocalFree(errMsg);
}

// To do: 2023/08/13
// 增加一个DLL export 函数，用于接受const char* 参数，参数表明被注入的程序keyword，用于区分对应的shared memory
// done: 2023/08/23
DLL_API long __stdcall SetHook(DWORD processId, size_t size)
{
    LOG(INFO) << "SetHook";
    // 需要在这里处理 SM, Mutex相关的初始化，用一个对像来表示
    std::stringstream ss;
    ss << std::string("Capture_")  << processId;
    //global_sh = std::make_shared<ScreenShotHook>(ss.str(), size);
    //global_sh->start(); 
    g_ipcrw = std::make_shared<IPCRW>(ss.str(), size);
    g_ipcrw->start(); 

    cus_thread = std::thread(customer);

    // 加入钩子
    orig_SwapBuffers = (SwapBuffersType)GetProcAddress(GetModuleHandle("gdi32.dll"), "SwapBuffers");
    if (orig_SwapBuffers)
    {
        LOG(INFO) << "Begin SwapBuffers Hook with SharedMemory";
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)orig_SwapBuffers, hooked_SwapBuffers_SharedMemory);
        DetourTransactionCommit();
    }

    return 0;
}

DLL_API long __stdcall ReleaseHook()
{
    terminate_flag = true; 
    // 重置钩子
    if (orig_SwapBuffers)
    {
        LOG(INFO) << "Release SwapBuffers Hook with SharedMemory";
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)orig_SwapBuffers, hooked_SwapBuffers_SharedMemory);
        DetourTransactionCommit();
    }

    // 释放资源
    //global_sh.reset(); 
    g_ipcrw.reset(); 

    return 0;
}


/*
* brief: DLL入口
*/

INITIALIZE_EASYLOGGINGPP
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    el::Configurations conf;
    conf.setToDefault();
    conf.set(el::Level::Global, el::ConfigurationType::Filename, R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\tests\inject\dll.log)");  // 设置日志文件的路径
    el::Loggers::reconfigureLogger("default", conf); // 重新配置默认的 logger
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break; 
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH: 
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}