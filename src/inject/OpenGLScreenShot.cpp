#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include "detours.h"
#include "shm_data.h"
#include "hook.h"

// global variable

typedef BOOL(WINAPI* SwapBuffersType)(HDC hdc);
SwapBuffersType orig_SwapBuffers = NULL; 
ScreenShotHook* global_sh; 



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
BOOL WINAPI hooked_SwapBuffers_SaveBMP(HDC hdc)
{
    static bool saved = FALSE;
    if (!saved) {
        BYTE* pixels = nullptr;
        int height = 0, width = 0; 
        GetWinPixels(hdc, reinterpret_cast<LPARAM>(&pixels), width, height);


		MessageBoxA(0, "get frame", "Injected", MB_ICONEXCLAMATION);
        // 保存为BMP
        SaveToBMP(pixels, width, height, R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\tests\inject\screenshot.bmp)");
        
        MessageBoxA(0, "saved", "Injected", MB_ICONEXCLAMATION);

        delete[] pixels;
        saved = TRUE;
    }

    return orig_SwapBuffers(hdc);
}

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
    memcpy(gloabl_sh->shm->data<BYTE*>() + sizeof(shm_header), pixels, width * height * 3);

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

#include <map>
// hooked_SwapBuffers 注册表
std::map<const char*, SwapBuffersType> SwapBuffers_register = { 
    {"SharedMemory",hooked_SwapBuffers_SharedMemory},
    {"SaveToFile", hooked_SwapBuffers_SaveBMP}
};


// To do: 2023/08/13
// 增加一个DLL export 函数，用于接受const char* 参数，参数表明被注入的程序keyword，用于区分对应的shared memory
long __stdcall SetHook(HWND hwnd, const std::string& op)
{
    // 需要在这里处理 SM, Mutex相关的初始化，用一个对像来表示
    std::stringstream ss;
    ss << hwnd << "_" << "CaptureBuffer";
    global_sh = new ScreenShotHook(ss.str());

    // 加入钩子
    orig_SwapBuffers = (SwapBuffersType)GetProcAddress(GetModuleHandle("gdi32.dll"), "SwapBuffers");
    if (orig_SwapBuffers)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)orig_SwapBuffers, hooked_SwapBuffers_register[op]);
        DetourTransactionCommit();
    }
}

long __stdcall ReleaseHook()
{
    // 重置钩子
    if (orig_SwapBuffers)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)orig_SwapBuffers, hooked_SwapBuffers);
        DetourTransactionCommit();
    }

    // 释放资源
    delete global_sh; 
}


/*
* brief: DLL入口
*/
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