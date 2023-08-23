#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include "detours.h"
#include "shm_data.h"
#include "screenshot.h"
#include "shared_memory.h"
#include "mutex.h"

// global variable

typedef BOOL(WINAPI* SwapBuffersType)(HDC hdc);
SwapBuffersType orig_SwapBuffers = NULL; 

// std::mutex is intra-process mutex without "name" identify,so need a inter-process mutex here
// every dll need a global name to identify which process is been injected
MutexSingleton* mutex;
SharedMemorySingleton* shm;


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

    Lock lock(mutex);

    // need try finally later
    // �ͻ������֪�������˶��٣�д��header structure
    memcpy(shm->data<BYTE*>(), &shm_header, sizeof(shm_header));
    memcpy(shm->data<BYTE*>() + sizeof(shm_header), pixels, width * height * 3);
    lock.unlock();

    delete[] pixels;
    return orig_SwapBuffers(hdc);
}

BOOL AllocSharedMemory(void)
{
    const char* shm_name = "CaptuerBuffer";
    const int MaxShmSize = 2560 * 1600 * 3;
    shm = SharedMemorySingleton::Instance(shm_name, MaxShmSize);
    return shm->open(); 
}

BOOL AllocMutex(void)
{
    const char* shm_name = "CaptuerBufferMutex";
    mutex =  MutexSingleton::Instance(shm_name); 
    return mutex->open(); 
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
std::map<const char*, SwapBuffersType> SwapBuffers_register = { 
    {"SharedMemory",hooked_SwapBuffers_SharedMemory},
    {"SaveToFile", hooked_SwapBuffers_SaveBMP}
};


// To do: 2023/08/13
// ����һ��DLL export ���������ڽ���const char* ����������������ע��ĳ���keyword���������ֶ�Ӧ��shared memory

/*
* brief: DLL���,ע���ʱ��ʼ���乲���ڴ�
*/
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MessageBoxA(0, "DLL process has been injected!", "Injected", MB_ICONEXCLAMATION);
        // ���乲���ڴ�
        
        if(!AllocSharedMemory())
            MessageBoxA(0, "Alloc shared memory failed", "Fail", MB_ICONEXCLAMATION);

        if (!AllocMutex())
        {
            PrintLastError();
            MessageBoxA(0, "Create Mutex fail", "OK1", MB_ICONEXCLAMATION);
        }
            

         
        
        orig_SwapBuffers = (SwapBuffersType)GetProcAddress(GetModuleHandle("gdi32.dll"), "SwapBuffers");
        if (orig_SwapBuffers)
        {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)orig_SwapBuffers, hooked_SwapBuffers_SharedMemory);
            DetourTransactionCommit();
        }
        else
        {
            MessageBoxA(0, "SwapBuffer not found!", "Fail", MB_ICONEXCLAMATION);
        }
        
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break; 
    case DLL_PROCESS_DETACH:

        break;
    }
    return TRUE;
}