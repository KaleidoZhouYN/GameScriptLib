#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include "detours.h"
#include "shm_data.h"
#include "hook.h"
#include <sstream>

// singleton static variable define
std::map<std::string, MutexSingleton*> MutexSingleton::_singleton = {};
MutexSingleton::GarbageCollector MutexSingleton::gc;
std::map<std::string, SharedMemorySingleton*> SharedMemorySingleton::_singleton = {};
SharedMemorySingleton::GarbageCollector SharedMemorySingleton::gc;

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

    // ʹ��glReadPixels��ȡ��ǰ֡����������
    glReadBuffer(GL_BACK);  // ����Ϊ��ȡ�󻺳���
    glReadPixels(0, 0, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, *pixels);  // ע��: ʹ��GL_BGR_EXT��ΪBMP�����ݲ���,BMPΪBGR,openGL��׼��ʽΪRGB
    return TRUE; 
}

/* 
* @brief: swapbuffer ���Ӻ���, �����䱣����������BMP·��
* @param: hdc ���ڵ� DC handle
*/
BOOL WINAPI hooked_SwapBuffers_SaveBMP(HDC hdc)
{
    static bool saved = FALSE;
    if (!saved) {
        BYTE* pixels = nullptr;
        int height = 0, width = 0; 
        GetWinPixels(hdc, reinterpret_cast<LPARAM>(&pixels), width, height);


		MessageBoxA(0, "get frame", "Injected", MB_ICONEXCLAMATION);
        // ����ΪBMP
        SaveToBMP(pixels, width, height, R"(C:\Users\zhouy\source\repos\GameScriptLib\out\build\x64-debug\tests\inject\screenshot.bmp)");
        
        MessageBoxA(0, "saved", "Injected", MB_ICONEXCLAMATION);

        delete[] pixels;
        saved = TRUE;
    }

    return orig_SwapBuffers(hdc);
}

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

    Lock lock(global_sh->mutex);

    // need try finally later
    // �ͻ������֪�������˶��٣�д��header structure
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
        0, // Ĭ������
        (LPSTR)&errMsg,
        0,
        NULL
    );

    MessageBoxA(0, (LPSTR)errMsg, "fail", MB_ICONEXCLAMATION);

    LocalFree(errMsg);
}

#include <map>
// hooked_SwapBuffers ע���
std::map<std::string, SwapBuffersType> SwapBuffers_register = { 
    {"SharedMemory",hooked_SwapBuffers_SharedMemory},
    {"SaveToFile", hooked_SwapBuffers_SaveBMP}
};


// To do: 2023/08/13
// ����һ��DLL export ���������ڽ���const char* ����������������ע��ĳ���keyword���������ֶ�Ӧ��shared memory
// done: 2023/08/23
extern "C" __declspec(dllexport) long __stdcall SetHook(DWORD processId, const size_t size)
{
    // ��Ҫ�����ﴦ�� SM, Mutex��صĳ�ʼ������һ����������ʾ
    std::stringstream ss;
    ss << std::string("CaptureBuffer_")  << processId;
    global_sh = new ScreenShotHook(ss.str(), size);
    global_sh->start(); 

    // ���빳��
    orig_SwapBuffers = (SwapBuffersType)GetProcAddress(GetModuleHandle("gdi32.dll"), "SwapBuffers");
    if (orig_SwapBuffers)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)orig_SwapBuffers, hooked_SwapBuffers_SharedMemory);
        DetourTransactionCommit();
    }

    return 0;
}

extern "C" __declspec(dllexport) long __stdcall ReleaseHook(DWORD processId)
{
    // ���ù���
    if (orig_SwapBuffers)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)orig_SwapBuffers, hooked_SwapBuffers_SharedMemory);
        DetourTransactionCommit();
    }

    // �ͷ���Դ
    delete global_sh; 

    return 0;
}


/*
* brief: DLL���
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