#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include "detours.h"
#include "shm_data.h"
#include "hook.h"
#include <sstream>
#define ELPP_THREAD_SAFE
#include "easylogging++.h"


// global variable
typedef BOOL(WINAPI* SwapBuffersType)(HDC hdc);
SwapBuffersType orig_SwapBuffers = NULL; 
std::shared_ptr<ScreenShotHook> global_sh; 


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

/* 
* @brief: swapbuffer 钩子函数, 并将其保存至给定的BMP路径
* @param: hdc 窗口的 DC handle
*/

/*
@brief: swapbuffer的钩子函数, 将frame binary data 保存进入shared memory
@param: hdc, 窗口的Device Context Handle
*/
BOOL WINAPI hooked_SwapBuffers_SharedMemory(HDC hdc)
{
    BYTE* pixels = nullptr;
    int height = 0, width = 0;
    GetWinPixels(hdc, reinterpret_cast<LPARAM>(&pixels), width, height);

    SharedDataHeader shm_header = { width, height, 3 };

    Lock lock(global_sh->mutex);

    // need try finally later
    // 客户端如何知道保存了多少？写入header structure
    memcpy(global_sh->shm->data<BYTE*>(), &shm_header, sizeof(shm_header));
    memcpy(global_sh->shm->data<BYTE*>() + sizeof(shm_header), pixels, width * height * 3);

    // global_sh->shm->writebuffer(0, &shm_header, sizeof(shm_header)); 
    // global_sh->shm->writebuffer(sizeof(shm_header), pixels, width * height * 3); 
    lock.unlock();

    delete[] pixels;
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
DLL_API long __stdcall SetHook(DWORD processId, const size_t size)
{
    // 需要在这里处理 SM, Mutex相关的初始化，用一个对像来表示
    std::stringstream ss;
    ss << std::string("CaptureBuffer_")  << processId;
    global_sh = std::make_shared<ScreenShotHook>(ss.str(), size);
    global_sh->start(); 

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

DLL_API long __stdcall ReleaseHook(DWORD processId)
{
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
    global_sh.reset(); 

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
    conf.set(el::Level::Global, el::ConfigurationType::Filename, "dll.log");  // 设置日志文件的路径
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