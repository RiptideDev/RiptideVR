#include <Windows.h>
#include <stdexcept>
#include "RenderDevice.h"
#include "DeviceContext.h"
#include "EngineFactoryD3D11.h"
#include "OpenVRInterface.h"

using namespace Diligent;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    // register window class
    WNDCLASS wc      = {};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = L"VRWindowClass";
    RegisterClass(&wc);

    // create hidden window
    HWND hwnd = CreateWindowEx(0, L"VRWindowClass", L"SIGMA", WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
                               nullptr, nullptr, hInstance, nullptr);
    ShowWindow(hwnd, SW_SHOW);

    try
    {
        // initialize engine

        RefCntAutoPtr<IRenderDevice>  pDevice;
        RefCntAutoPtr<IDeviceContext> pContext;

        EngineD3D11CreateInfo EngineCI;
        EngineCI.GraphicsAPIVersion = {11, 0};
        auto GetEngineFactoryD3D11Func = LoadGraphicsEngineD3D11();
        if (!GetEngineFactoryD3D11Func)
            throw std::runtime_error("Failed to load D3D11 engine factory");

        // get the factory object from the function pointer
        IEngineFactoryD3D11* pEngineFactoryD3D11 = GetEngineFactoryD3D11Func();
        if (!pEngineFactoryD3D11)
            throw std::runtime_error("Failed to obtain D3D11 engine factory");

        // get the device and contexts from the engine factory
        pEngineFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &pDevice, &pContext);
        if (!pDevice || !pContext)
            throw std::runtime_error("Failed to create D3D11 device and context");

        // initialize openvr
        OpenVRInterface vrInterface(pDevice, pContext);
        vrInterface.Initialize();

        // main loop
        MSG msg = {};
        while (true)
        {
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                    return 0;

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            vrInterface.RenderFrame();
        }
    }
    catch (const std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    // cleanup
    DestroyWindow(hwnd);
    UnregisterClass(L"VRWindowClass", hInstance);
    return 0;
}