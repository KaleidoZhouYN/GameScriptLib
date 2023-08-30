#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include "detours.h"
#include "frame_info.h"
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

    // ʹ��glReadPixels��ȡ��ǰ֡����������
    glReadBuffer(GL_BACK);  // ����Ϊ��ȡ�󻺳���
    glReadPixels(0, 0, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, *pixels);  // ע��: ʹ��GL_BGR_EXT��ΪBMP�����ݲ���,BMPΪBGR,openGL��׼��ʽΪRGB
    return TRUE; 
}

/* 
* @brief: swapbuffer ���Ӻ���, �����䱣����������BMP·��
* @param: hdc ���ڵ� DC handle
*/

/*
@brief: swapbuffer�Ĺ��Ӻ���, ��frame binary data �������shared memory
@param: hdc, ���ڵ�Device Context Handle
*/
BOOL WINAPI hooked_SwapBuffers_SharedMemory(HDC hdc)
{
    BYTE* pixels = nullptr;
    int height = 0, width = 0;
    GetWinPixels(hdc, reinterpret_cast<LPARAM>(&pixels), width, height);

    SharedDataHeader shm_header = { width, height, 3 };

    Lock lock(global_sh->get_mutex());

    // need try finally later
    // �ͻ������֪�������˶��٣�д��header structure
    memcpy(global_sh->data<BYTE*>(), &shm_header, sizeof(shm_header));
    memcpy(global_sh->data<BYTE*>() + sizeof(shm_header), pixels, width * height * 3);

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
        0, // Ĭ������
        (LPSTR)&errMsg,
        0,
        NULL
    );

    MessageBoxA(0, (LPSTR)errMsg, "fail", MB_ICONEXCLAMATION);

    LocalFree(errMsg);
}

// To do: 2023/08/13
// ����һ��DLL export ���������ڽ���const char* ����������������ע��ĳ���keyword���������ֶ�Ӧ��shared memory
// done: 2023/08/23
DLL_API long __stdcall SetHook(DWORD processId, const size_t size)
{
    // ��Ҫ�����ﴦ�� SM, Mutex��صĳ�ʼ������һ����������ʾ
    std::stringstream ss;
    ss << std::string("Capture_")  << processId;
    global_sh = std::make_shared<ScreenShotHook>(ss.str(), size);
    global_sh->start(); 

    // ���빳��
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
    // ���ù���
    if (orig_SwapBuffers)
    {
        LOG(INFO) << "Release SwapBuffers Hook with SharedMemory";
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)orig_SwapBuffers, hooked_SwapBuffers_SharedMemory);
        DetourTransactionCommit();
    }

    // �ͷ���Դ
    global_sh.reset(); 

    return 0;
}


/*
* brief: DLL���
*/

INITIALIZE_EASYLOGGINGPP
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    el::Configurations conf;
    conf.setToDefault();
    conf.set(el::Level::Global, el::ConfigurationType::Filename, R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\tests\inject\dll.log)");  // ������־�ļ���·��
    el::Loggers::reconfigureLogger("default", conf); // ��������Ĭ�ϵ� logger
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