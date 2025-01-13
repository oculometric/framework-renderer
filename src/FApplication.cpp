#include "FApplication.h"

#include <string>
#include <queue>
#include <thread>

#include "FGraphicsEngine.h"
#include "FPhysicsEngine.h"
#include "FJsonParser.h"
#include "FScene.h"
#include "FResourceManager.h"
#include "FDebug.h"

using namespace std;

FApplication* application = nullptr;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_MOVE:
    case WM_SIZE:
        if (application == nullptr) break;
        if (application->getWindow() != hWnd) break;
            application->updateWindowSize();
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

HRESULT FApplication::initialise(HINSTANCE hInstance, int nShowCmd)
{
    HRESULT hr = S_OK;

    engine = new FGraphicsEngine(this);
    FResourceManager::set(new FResourceManager(engine));
    
    hr = createWindowHandle(hInstance, nShowCmd);
    if (FAILED(hr)) return E_FAIL;

    FDebug::set(new FDebug(window_handle));

    hr = createD3DDevice();
    if (FAILED(hr)) return E_FAIL;

    hr = engine->initialise();
    if (FAILED(hr)) return E_FAIL;

    application = this;
    updateWindowSize();

    time_keeper = chrono::high_resolution_clock::now();

    physics_engine = new FPhysicsEngine(this);

    return hr;
}

HRESULT FApplication::createWindowHandle(HINSTANCE hInstance, int nCmdShow)
{
    const wchar_t* window_name  = L"SpaceDemoProject";

    WNDCLASSW wnd_class = { };
    wnd_class.style = 0;
    wnd_class.lpfnWndProc = WndProc;
    wnd_class.cbClsExtra = 0;
    wnd_class.cbWndExtra = 0;
    wnd_class.hInstance = 0;
    wnd_class.hIcon = 0;
    wnd_class.hCursor = 0;
    wnd_class.hbrBackground = 0;
    wnd_class.lpszMenuName = 0;
    wnd_class.lpszClassName = window_name;

    RegisterClassW(&wnd_class);

    RECT rc = { 0,0, window_width, window_height };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    window_handle = CreateWindow(window_name, window_name, WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME, CW_USEDEFAULT, CW_USEDEFAULT, rc.right-rc.left, rc.bottom-rc.top, nullptr, nullptr, hInstance, nullptr);
    window_x = rc.left;
    window_y = rc.top;

    const wchar_t* info_window_name = L"Framework Info";
    info_window_handle = CreateWindow(L"STATIC", info_window_name, WS_VISIBLE | WS_OVERLAPPED | SS_LEFT | WS_POPUP, rc.right + 200, rc.top + 200, 300, 400, window_handle, nullptr, hInstance, nullptr);

    return S_OK;
}

HRESULT FApplication::createD3DDevice()
{
    HRESULT hr = S_OK;

    D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
    };

    ID3D11Device* base_device;
    ID3D11DeviceContext* base_device_context;

    DWORD create_device_flags = 0;
#ifdef _DEBUG
    create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT | create_device_flags, feature_levels, ARRAYSIZE(feature_levels), D3D11_SDK_VERSION, &base_device, nullptr, &base_device_context);
    if (FAILED(hr)) return hr;

    ///////////////////////////////////////////////////////////////////////////////////////////////

    hr = base_device->QueryInterface(__uuidof(ID3D11Device), reinterpret_cast<void**>(&device));
    hr = base_device_context->QueryInterface(__uuidof(ID3D11DeviceContext), reinterpret_cast<void**>(&immediate_context));

    base_device->Release();
    base_device_context->Release();

    ///////////////////////////////////////////////////////////////////////////////////////////////

    hr = device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgi_device));
    if (FAILED(hr)) return hr;

    IDXGIAdapter* dxgi_adapter;
    hr = dxgi_device->GetAdapter(&dxgi_adapter);
    hr = dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgi_factory));
    dxgi_adapter->Release();

    return S_OK;
}

FApplication* FApplication::get()
{
    return application;
}

void FApplication::updateWindowSize()
{
    RECT r;
    GetClientRect(window_handle, &r);
    int new_width = (int)(r.right - r.left);
    int new_height = (int)(r.bottom - r.top);
    if (new_width != window_width || new_height != window_height)
        needs_viewport_resize = true;
    window_width = new_width;
    window_height = new_height;

    GetWindowRect(window_handle, &r);
    window_x = r.left;
    window_y = r.top;
}

void FApplication::updateStats(wstring str)
{
    SetWindowText(info_window_handle, str.c_str());
}

void FApplication::update()
{
    chrono::steady_clock::time_point now = chrono::high_resolution_clock::now();
    chrono::duration<float> delta = now - time_keeper;

    float delta_time = delta.count();
    mean_frame_time = (0.5f * delta_time) + (0.5f * mean_frame_time);
    float fps = 1.0f / mean_frame_time;

    string new_title = "FrameworkRenderer - " + scene->name + " [ " + to_string(fps) + "fps, " + to_string(mean_frame_time) + "ms ]";
    SetWindowTextA(window_handle, new_title.c_str());

    time_keeper = now;

    total_time += delta_time;

    if (scene)
        scene->update(delta_time);
}

void FApplication::draw()
{
    engine->draw();
}

FApplication::~FApplication()
{
    delete physics_engine;
    delete engine;

    if (immediate_context) immediate_context->Release();
    if (device) device->Release();
    if (dxgi_device) dxgi_device->Release();
    if (dxgi_factory) dxgi_factory->Release();

    delete FDebug::get();
}
