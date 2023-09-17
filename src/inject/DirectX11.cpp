#include<d3d11.h>
#include<windows.h>
#include "detours.h"
#include "frame_info.h"
#include "ipcrw.h"
#include <sstream>
#include <functional>

#include "easylogging++.h"
#define ELPP_THREAD_SAFE

//define Image byte format
constexpr int IBF_R8G8B8A8 = 0;
constexpr int IBF_B8G8R8A8 = 1;
constexpr int IBF_R8G8B8 = 2;

typedef HRESULT(WINAPI* BufferType)(IDXGISwapChain* pswapchain, UINT x1, UINT x2);
BufferType orig_address = NULL; 
std::shared_ptr<IPCRW> g_ipcrw; 
typedef uint64_t uint150_t;
uint150_t* g_methodsTable = NULL;

#define DLL_API extern "C" __declspec(dllexport)

DXGI_FORMAT GetDxgiFormat(DXGI_FORMAT format) {
    if (format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB) {
        return DXGI_FORMAT_B8G8R8A8_UNORM;
    }
    if (format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) {
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    }
    return format;
}

void WINAPI hooked_Impl(IDXGISwapChain* swapchain, UINT x1, UINT x2)
{
    using Texture2D = ID3D11Texture2D*;
    HRESULT hr = 0;
    IDXGIResource* backbufferptr = nullptr;
    ID3D11Resource* backbuffer = nullptr;
    Texture2D textDst = nullptr;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    hr = swapchain->GetBuffer(0, __uuidof(IDXGIResource), (void**)&backbufferptr);
    if (hr < 0) {
        LOG(INFO) << "pswapchain->GetBuffer,error code=" << hr;
        return; 
    }
    hr = backbufferptr->QueryInterface(__uuidof(ID3D11Resource), (void**)&backbuffer);
    hr = swapchain->GetDevice(__uuidof(ID3D11Device), (void**)&device);
    DXGI_SWAP_CHAIN_DESC desc;
    hr = swapchain->GetDesc(&desc);

    D3D11_TEXTURE2D_DESC textDesc = { };
    textDesc.Format = GetDxgiFormat(desc.BufferDesc.Format);
    textDesc.Width = desc.BufferDesc.Width;
    textDesc.Height = desc.BufferDesc.Height;
    textDesc.MipLevels = 1;
    textDesc.ArraySize = 1;
    textDesc.SampleDesc.Count = 1;
    textDesc.Usage = D3D11_USAGE_STAGING;
    textDesc.BindFlags = 0;
    textDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    textDesc.MiscFlags = 0;

    hr = device->CreateTexture2D(&textDesc, nullptr, &textDst);

    device->GetImmediateContext(&context);
    context->CopyResource(textDst, backbuffer);
    D3D11_MAPPED_SUBRESOURCE mapSubres = { 0,0,0 };
    hr = context->Map(textDst, 0, D3D11_MAP_READ, 0, &mapSubres);
    int fmt = IBF_R8G8B8A8;
    if (textDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM ||
        textDesc.Format == DXGI_FORMAT_B8G8R8X8_UNORM ||
        textDesc.Format == DXGI_FORMAT_B8G8R8A8_TYPELESS ||
        textDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB ||
        textDesc.Format == DXGI_FORMAT_B8G8R8X8_TYPELESS ||
        textDesc.Format == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB)
    {
        fmt = IBF_B8G8R8A8;
    }

    FrameInfo frame;
    SharedDataHeader shm_header = { textDesc.Width,textDesc.Height, 4 };
    frame.header = shm_header;

    // 考虑copy buffer
    frame.new_buffer();

    if (fmt == IBF_B8G8R8A8)
    {
        memcpy(frame.buffer, mapSubres.pData, frame.get_buffer_size());
    }
    if (fmt == IBF_R8G8B8A8)
    {
        BYTE* dst = frame.buffer;
        BYTE* src = static_cast<BYTE*>(mapSubres.pData);
        for (int i = 0; i < frame.header.height; i++)
            for (int j = 0; j < frame.header.width; j++)
            {
                dst[0] = src[2];
                dst[2] = src[0];
                dst += 4;
                src += 4;
            }
    }
    //frame.buffer = static_cast<byte*>(mapSubres.pData);

    auto boundFn = std::bind(&FrameInfo::write, &frame, std::placeholders::_1);
    g_ipcrw->write_to_sm(boundFn);
    LOG(INFO) << "write to sm success";
}

long WINAPI hooked_SharedMemory(IDXGISwapChain* swapchain, UINT x1, UINT x2)
{
    LOG(INFO) << "call swapchain";
    hooked_Impl(swapchain, x1, x2);
    return orig_address(swapchain, x1, x2);
}

DLL_API long __stdcall SetHook(DWORD processId, size_t size)
{
    LOG(INFO) << "SetHook";
    // 需要在这里处理 SM, Mutex相关的初始化，用一个对像来表示
    std::stringstream ss;
    ss << std::string("Capture_") << processId;
    //global_sh = std::make_shared<ScreenShotHook>(ss.str(), size);
    //global_sh->start(); 
    g_ipcrw = std::make_shared<IPCRW>(ss.str(), size);
    g_ipcrw->start();

    HMODULE libD3D11 = ::GetModuleHandle("d3d11.dll");
    void* D3D11CreateDeviceAndSwapChain = ::GetProcAddress(libD3D11, "D3D11CreateDeviceAndSwapChain");
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0 };

    DXGI_RATIONAL refreshRate;
    refreshRate.Numerator = 60;
    refreshRate.Denominator = 1;

    DXGI_MODE_DESC bufferDesc;
    bufferDesc.Width = 100;
    bufferDesc.Height = 100;
    bufferDesc.RefreshRate = refreshRate;
    bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    DXGI_SAMPLE_DESC sampleDesc;
    sampleDesc.Count = 1;
    sampleDesc.Quality = 0;

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    swapChainDesc.BufferDesc = bufferDesc;
    swapChainDesc.SampleDesc = sampleDesc;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.Windowed = 1;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    IDXGISwapChain* swapChain;
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    ((long(__stdcall*)(
        IDXGIAdapter*,
        D3D_DRIVER_TYPE,
        HMODULE,
        UINT,
        const D3D_FEATURE_LEVEL*,
        UINT,
        UINT,
        const DXGI_SWAP_CHAIN_DESC*,
        IDXGISwapChain**,
        ID3D11Device**,
        D3D_FEATURE_LEVEL*,
        ID3D11DeviceContext**))(D3D11CreateDeviceAndSwapChain))(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, 2, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, &featureLevel, &context);
    g_methodsTable = (uint150_t*)::calloc(205, sizeof(uint150_t));
    ::memcpy(g_methodsTable, *(uint150_t**)swapChain, 18 * sizeof(uint150_t));
    ::memcpy(g_methodsTable + 18, *(uint150_t**)device, 43 * sizeof(uint150_t));
    ::memcpy(g_methodsTable + 18 + 43, *(uint150_t**)context, 144 * sizeof(uint150_t));
    swapChain->Release();
    swapChain = NULL;
    device->Release();
    device = NULL;
    context->Release();
    context = NULL;


    // 加入钩子
    orig_address = (BufferType)(g_methodsTable[8]);
    if (orig_address)
    {
        LOG(INFO) << "Begin Hook with SharedMemory";
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        if (DetourAttach(&(PVOID&)orig_address, hooked_SharedMemory) != NO_ERROR)
        {
            LOG(INFO) << "bind error";
        }
        DetourTransactionCommit();
    }

    return 0;
}

DLL_API long __stdcall ReleaseHook()
{
    // 重置钩子
    if (orig_address)
    {
        LOG(INFO) << "Release Hook with SharedMemory";
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)orig_address, hooked_SharedMemory);
        DetourTransactionCommit();
    }

    // 释放资源
    //global_sh.reset(); 
    g_ipcrw.reset();

    return 0;
}

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