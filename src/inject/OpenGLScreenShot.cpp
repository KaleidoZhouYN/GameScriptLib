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
std::shared_ptr<IPCRW> g_ipcrw; 


#define DLL_API extern "C" __declspec(dllexport)


//buffer operate from origin buffer to opencv::BGR
void FmtProcess(FrameInfo* pframe, RENDER_TYPE render_type, IBF fmt)
{
    byte* buffer = pframe->buffer;
    int width = pframe->header.width, height = pframe->header.height, channel = pframe->header.channel;

    if (render_type == RENDER_TYPE::OPENGL)
    {
        // fliplr
        /*
        std::allocator<byte> byte_alloc;
        size_t copy_size = width * channel;
        byte* tmp = byte_alloc.allocate(copy_size);
        byte* src_address = nullptr;
        byte* dst_address = nullptr;
        for (int i = 0; i < height / 2; i++)
        {
            src_address = buffer + i * copy_size; 
            dst_address = buffer + (height - i -1) * copy_size; 
            std::memcpy(tmp,src_address, copy_size);
            std::memcpy(src_address, dst_address, copy_size);
            std::memcpy(dst_address, tmp, copy_size); 
        }
        */
        size_t copy_size = width * channel;
        for (int i = 0; i < height / 2; i++)
        {
            byte* src_address = buffer + i * copy_size; 
            byte* dst_address = buffer + (height - i - 1) * copy_size; 
            std::swap_ranges(src_address, src_address+ copy_size, dst_address);
        }
        return; 
    }
    if (fmt == IBF::R8G8B8 || fmt == IBF::R8G8B8A8)
    {
        // RGB8BGR
        byte* src = buffer; 
        size_t copy_size = width * channel;
        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++)
            { 
                std::swap(src[0], src[2]);
                src += channel; 
            }
        return; 
    }
    return;
}

// To do : Change width, heigth to RParam
/*
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
*/

BOOL GetWinPixels(HDC hdc, LPARAM lParam)
{
    HWND hwnd = WindowFromDC(hdc);
    RECT rect; 
    GetClientRect(hwnd, &rect);
    FrameInfo* pframe = reinterpret_cast<FrameInfo*>(lParam);
    pframe->header.width = rect.right - rect.left; 
    pframe->header.height = rect.bottom - rect.top; 
    pframe->header.channel = 3; 

    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, pframe->header.width, pframe->header.height, GL_BGR_EXT, GL_UNSIGNED_BYTE, pframe->buffer);  // ע��: ʹ��GL_BGR_EXT��ΪBMP�����ݲ���,BMPΪBGR,openGL��׼��ʽΪRGB
    return TRUE; 
}

DLL_API long __stdcall ReleaseHook();
/*
@brief: swapbuffer�Ĺ��Ӻ���, ��frame binary data �������shared memory
@param: hdc, ���ڵ�Device Context Handle
*/

std::mutex mtx; 
std::condition_variable cv; 
FrameInfo shared_frame; 
bool ready = false;  // ������ٻ���
std::thread cus_thread; 
std::atomic<bool> terminate_flag(false);

void WINAPI consumer()
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

// �����쳣��ʱ����ΰ�ȫ�ͷŹ���
BOOL WINAPI hooked_SwapBuffers_SharedMemory(HDC hdc)
{
    try {
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
        if (lock.try_lock())
        {
            //FrameInfo frame;
            int height = 0, width = 0;
            GetWinPixels(hdc, reinterpret_cast<LPARAM>(&shared_frame));
            //SharedDataHeader shm_header = { width, height, 3 };
            //shared_frame.header = shm_header;
            ready = true;
            cv.notify_all();
        }
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "�����쳣:" << e.what();
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
DLL_API long __stdcall SetHook(DWORD processId, size_t size)
{
    LOG(INFO) << "SetHook";
    // ��Ҫ�����ﴦ�� SM, Mutex��صĳ�ʼ������һ����������ʾ
    std::stringstream ss;
    ss << std::string("Capture_")  << processId;
    //global_sh = std::make_shared<ScreenShotHook>(ss.str(), size);
    //global_sh->start(); 
    g_ipcrw = std::make_shared<IPCRW>(ss.str(), size);
    g_ipcrw->start(); 

    cus_thread = std::thread(consumer);

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

DLL_API long __stdcall ReleaseHook()
{
    terminate_flag = true; 
    // ���ù���
    if (orig_SwapBuffers)
    {
        LOG(INFO) << "Release SwapBuffers Hook with SharedMemory";
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)orig_SwapBuffers, hooked_SwapBuffers_SharedMemory);
        DetourTransactionCommit();
        orig_SwapBuffers = nullptr; 
        // �ͷ���Դ 
        g_ipcrw.reset();
    } 

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