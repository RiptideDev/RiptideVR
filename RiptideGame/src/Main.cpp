#include "EngineFactoryD3D11.h"
#include "EngineFactoryD3D12.h"
#include "EngineFactoryOpenGL.h"
#include "EngineFactoryVk.h"
#include <RefCntAutoPtr.hpp>

using namespace Diligent;

RefCntAutoPtr<IRenderDevice>  m_pDevice;
RefCntAutoPtr<IDeviceContext> m_pImmediateContext;
RefCntAutoPtr<ISwapChain>     m_pSwapChain;

void InitializeDiligentEngine(HWND hWnd, RENDER_DEVICE_TYPE deviceType)
{
    SwapChainDesc SCDesc;
    switch (deviceType)
    {
        case RENDER_DEVICE_TYPE_D3D11:
        {
            EngineD3D11CreateInfo EngineCI;
#if ENGINE_DLL
            // Load the dll and import GetEngineFactoryD3D11() function
            auto* GetEngineFactoryD3D11 = LoadGraphicsEngineD3D11();
#endif
            auto* pFactoryD3D11 = GetEngineFactoryD3D11();
            pFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &m_pDevice, &m_pImmediateContext);
            Win32NativeWindow Window{hWnd};
            pFactoryD3D11->CreateSwapChainD3D11(m_pDevice, m_pImmediateContext, SCDesc,
                                                FullScreenModeDesc{}, Window, &m_pSwapChain);
        }
        break;

        case RENDER_DEVICE_TYPE_D3D12:
        {
#if ENGINE_DLL
            // Load the dll and import GetEngineFactoryD3D12() function
            auto GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
#endif
            EngineD3D12CreateInfo EngineCI;

            auto* pFactoryD3D12 = GetEngineFactoryD3D12();
            pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &m_pDevice, &m_pImmediateContext);
            Win32NativeWindow Window{hWnd};
            pFactoryD3D12->CreateSwapChainD3D12(m_pDevice, m_pImmediateContext, SCDesc,
                                                FullScreenModeDesc{}, Window, &m_pSwapChain);
        }
        break;

        case RENDER_DEVICE_TYPE_GL:
        {
#if EXPLICITLY_LOAD_ENGINE_GL_DLL
            // Load the dll and import GetEngineFactoryOpenGL() function
            auto GetEngineFactoryOpenGL = LoadGraphicsEngineOpenGL();
#endif
            auto* pFactoryOpenGL = GetEngineFactoryOpenGL();

            EngineGLCreateInfo EngineCI;
            EngineCI.Window.hWnd = hWnd;

            pFactoryOpenGL->CreateDeviceAndSwapChainGL(EngineCI, &m_pDevice, &m_pImmediateContext,
                                                       SCDesc, &m_pSwapChain);
        }
        break;

        case RENDER_DEVICE_TYPE_VULKAN:
        {
#if EXPLICITLY_LOAD_ENGINE_VK_DLL
            // Load the dll and import GetEngineFactoryVk() function
            auto GetEngineFactoryVk = LoadGraphicsEngineVk();
#endif
            EngineVkCreateInfo EngineCI;

            auto* pFactoryVk = GetEngineFactoryVk();
            pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &m_pDevice, &m_pImmediateContext);
            Win32NativeWindow Window{hWnd};
            pFactoryVk->CreateSwapChainVk(m_pDevice, m_pImmediateContext, SCDesc, Window, &m_pSwapChain);
        }
        break;

        default:
            std::cerr << "Unknown device type";
    }
}

LRESULT CALLBACK WIN32WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HWND CreateWindowWIN32(const char* title, int w, int h, HINSTANCE hInstance)
{
    const char *CLASS_NAME = "RiptideGameClass";

    WNDCLASSA wc = {};

    wc.lpfnWndProc   = WIN32WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,                           // Optional window styles.
        CLASS_NAME,                  // Window class
        title, // Window text
        WS_OVERLAPPEDWINDOW,         // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, w, h,

        NULL,      // Parent window
        NULL,      // Menu
        hInstance, // Instance handle
        NULL       // Additional application data
    );

    if (hwnd == NULL)
    {
        return nullptr;
    }
    ShowWindow(hwnd, SW_SHOW);
    return hwnd;
}

int WinMain(
    HINSTANCE           hInstance,
    HINSTANCE hPrevInstance,
    LPSTR               lpCmdLine,
    int                 nShowCmd)
{
    HWND hWnd = CreateWindowWIN32("riptide test", 640, 480, hInstance);
    InitializeDiligentEngine(hWnd, RENDER_DEVICE_TYPE_VULKAN);
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    // DestroyWindow(hWnd);
}

int main()
{
    
}