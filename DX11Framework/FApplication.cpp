#include "FApplication.h"
#include <string>
#include <queue>
#include "FScene.h"

//#define RETURNFAIL(x) if(FAILED(x)) return x;

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

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;

    // TODO: implement window resizeability
}

HRESULT FApplication::initialise(HINSTANCE hInstance, int nShowCmd)
{
    HRESULT hr = S_OK;

    hr = createWindowHandle(hInstance, nShowCmd);
    if (FAILED(hr)) return E_FAIL;

    hr = createD3DDevice();
    if (FAILED(hr)) return E_FAIL;

    hr = createSwapChainAndFrameBuffer();
    if (FAILED(hr)) return E_FAIL;

    hr = initShadersAndInputLayout();
    if (FAILED(hr)) return E_FAIL;

    hr = initVertexIndexBuffers();
    if (FAILED(hr)) return E_FAIL;

    hr = initPipelineVariables();
    if (FAILED(hr)) return E_FAIL;

    hr = initRunTimeData();
    if (FAILED(hr)) return E_FAIL;

    return hr;
}

HRESULT FApplication::createWindowHandle(HINSTANCE hInstance, int nCmdShow)
{
    const wchar_t* window_name  = L"DX11Framework";

    WNDCLASSW wnd_class;
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

    window_handle = CreateWindowExW(0, window_name, window_name, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,window_width, window_height, nullptr, nullptr, hInstance, nullptr);

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

HRESULT FApplication::createSwapChainAndFrameBuffer()
{
    HRESULT hr = S_OK;

    DXGI_SWAP_CHAIN_DESC1 swap_chain_descriptor;
    swap_chain_descriptor.Width = 0; // Defer to WindowWidth
    swap_chain_descriptor.Height = 0; // Defer to WindowHeight
    swap_chain_descriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //FLIP* modes don't support sRGB backbuffer
    swap_chain_descriptor.Stereo = FALSE;
    swap_chain_descriptor.SampleDesc.Count = 1;
    swap_chain_descriptor.SampleDesc.Quality = 0;
    swap_chain_descriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_descriptor.BufferCount = 2;
    swap_chain_descriptor.Scaling = DXGI_SCALING_STRETCH;
    swap_chain_descriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swap_chain_descriptor.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swap_chain_descriptor.Flags = 0;

    hr = dxgi_factory->CreateSwapChainForHwnd(device, window_handle, &swap_chain_descriptor, nullptr, nullptr, &swap_chain);
    if (FAILED(hr)) return hr;

    ///////////////////////////////////////////////////////////////////////////////////////////////

    ID3D11Texture2D* frame_buffer = nullptr;

    hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&frame_buffer));
    if (FAILED(hr)) return hr;

    D3D11_RENDER_TARGET_VIEW_DESC frame_buffer_descriptor = {};
    frame_buffer_descriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; //sRGB render target enables hardware gamma correction
    frame_buffer_descriptor.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    hr = device->CreateRenderTargetView(frame_buffer, &frame_buffer_descriptor, &render_target_view);

    D3D11_TEXTURE2D_DESC depth_buffer_descriptor = { };
    frame_buffer->GetDesc(&depth_buffer_descriptor);
    depth_buffer_descriptor.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_buffer_descriptor.BindFlags = D3D10_BIND_DEPTH_STENCIL;

    device->CreateTexture2D(&depth_buffer_descriptor, nullptr, &depth_stencil_buffer);
    device->CreateDepthStencilView(depth_stencil_buffer, nullptr, &depth_stencil_view);

    frame_buffer->Release();

    return hr;
}

HRESULT FApplication::initShadersAndInputLayout()
{
    HRESULT hr = S_OK;
    ID3DBlob* error_blob;

    DWORD shader_flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    shader_flags |= D3DCOMPILE_DEBUG;
#endif
    
    ID3DBlob* vertex_shader_blob;

    hr =  D3DCompileFromFile(L"SimpleShaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_main", "vs_5_0", shader_flags, 0, &vertex_shader_blob, &error_blob);
    if (FAILED(hr))
    {
        MessageBoxA(window_handle, (char*)error_blob->GetBufferPointer(), nullptr, ERROR);
        error_blob->Release();
        return hr;
    }

    hr = device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), nullptr, &vertex_shader);

    if (FAILED(hr)) return hr;

    D3D11_INPUT_ELEMENT_DESC input_element_descriptor[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hr = device->CreateInputLayout(input_element_descriptor, ARRAYSIZE(input_element_descriptor), vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), &input_layout);
    if (FAILED(hr)) return hr;

    ///////////////////////////////////////////////////////////////////////////////////////////////

    ID3DBlob* pixel_shader_blob;

    hr = D3DCompileFromFile(L"SimpleShaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_main", "ps_5_0", shader_flags, 0, &pixel_shader_blob, &error_blob);
    if (FAILED(hr))
    {
        MessageBoxA(window_handle, (char*)error_blob->GetBufferPointer(), nullptr, ERROR);
        error_blob->Release();
        return hr;
    }

    hr = device->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), nullptr, &pixel_shader);

    vertex_shader_blob->Release();
    pixel_shader_blob->Release();

    return hr;
}

HRESULT FApplication::initVertexIndexBuffers()
{
    // we don't need this anymore

    return S_OK;
}

HRESULT FApplication::initPipelineVariables()
{
    HRESULT hr = S_OK;

    //Input Assembler
    immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    immediate_context->IASetInputLayout(input_layout);

    //Rasterizer
    D3D11_RASTERIZER_DESC rasterizer_descriptor = {};
    rasterizer_descriptor.FillMode = D3D11_FILL_SOLID;
    rasterizer_descriptor.CullMode = D3D11_CULL_BACK;

    hr = device->CreateRasterizerState(&rasterizer_descriptor, &rasterizer_state);
    if (FAILED(hr)) return hr;

    // Debug Rasterizer
    D3D11_RASTERIZER_DESC debug_rasterizer_descriptor = {};
    debug_rasterizer_descriptor.FillMode = D3D11_FILL_WIREFRAME;
    debug_rasterizer_descriptor.CullMode = D3D11_CULL_NONE;

    hr = device->CreateRasterizerState(&debug_rasterizer_descriptor, &debug_rasterizer_state);
    if (FAILED(hr)) return hr;

    immediate_context->RSSetState(rasterizer_state);

    //Viewport Values
    viewport = { 0.0f, 0.0f, (float)window_width, (float)window_height, 0.0f, 1.0f };
    immediate_context->RSSetViewports(1, &viewport);

    //Constant Buffer
    D3D11_BUFFER_DESC constant_buffer_descriptor = {};
    constant_buffer_descriptor.ByteWidth = sizeof(ConstantBuffer);
    constant_buffer_descriptor.Usage = D3D11_USAGE_DYNAMIC;
    constant_buffer_descriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_descriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = device->CreateBuffer(&constant_buffer_descriptor, nullptr, &constant_buffer);
    if (FAILED(hr)) { return hr; }

    immediate_context->VSSetConstantBuffers(0, 1, &constant_buffer);
    immediate_context->PSSetConstantBuffers(0, 1, &constant_buffer);

    return S_OK;
}

HRESULT FApplication::initRunTimeData()
{
    // we don't need all this anymore
    return S_OK;
}

void FApplication::registerMesh(FMeshData* mesh_data)
{
    HRESULT hr = S_OK;

    FVertex* vertex_data = new FVertex[mesh_data->vertex_count];
    for (int i = 0; i < mesh_data->vertex_count; i++)
        vertex_data[i] = FVertex{ mesh_data->position[i], mesh_data->colour[i] };

    D3D11_BUFFER_DESC vertex_buffer_descriptor = {};
    vertex_buffer_descriptor.ByteWidth = sizeof(vertex_data);
    vertex_buffer_descriptor.Usage = D3D11_USAGE_IMMUTABLE;
    vertex_buffer_descriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertex_subresource_data = { vertex_data };

    hr = device->CreateBuffer(&vertex_buffer_descriptor, &vertex_subresource_data, &mesh_data->vertex_buffer_ptr);
    if (FAILED(hr)) return;

    uint16_t* index_data = mesh_data->indices;

    D3D11_BUFFER_DESC index_buffer_descriptor = { };
    index_buffer_descriptor.ByteWidth = sizeof(index_data);
    index_buffer_descriptor.Usage = D3D11_USAGE_IMMUTABLE;
    index_buffer_descriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA index_subresource_data = { index_data };

    hr = device->CreateBuffer(&index_buffer_descriptor, &index_subresource_data, &mesh_data->index_buffer_ptr);
    if (FAILED(hr)) return;
}

void FApplication::unregisterMesh(FMeshData* mesh_data)
{
    if (mesh_data->vertex_buffer_ptr) { mesh_data->vertex_buffer_ptr->Release(); mesh_data->vertex_buffer_ptr = nullptr; }
    if (mesh_data->index_buffer_ptr) { mesh_data->index_buffer_ptr->Release(); mesh_data->index_buffer_ptr = nullptr; }
}

FApplication::~FApplication()
{
    if (immediate_context) immediate_context->Release();
    if (device) device->Release();
    if (dxgi_device) dxgi_device->Release();
    if (dxgi_factory) dxgi_factory->Release();
    if (render_target_view) render_target_view->Release();
    if (depth_stencil_view) depth_stencil_view->Release();
    if (swap_chain) swap_chain->Release();

    if (rasterizer_state) rasterizer_state->Release();
    if (debug_rasterizer_state) debug_rasterizer_state->Release();
    if (vertex_shader) vertex_shader->Release();
    if (input_layout) input_layout->Release();
    if (pixel_shader) pixel_shader->Release();
    if (constant_buffer) constant_buffer->Release();
    if (depth_stencil_buffer) depth_stencil_buffer->Release();
    if (scene) delete scene;
}

void FApplication::update()
{
    static ULONGLONG frame_start = GetTickCount64();

    ULONGLONG frame_now = GetTickCount64();
    float delta_time = (frame_now - frame_start) / 1000.0f;
    frame_start = frame_now;

    static float total_time = 0.0f;
    total_time += delta_time;

    static bool is_debug_mode = false;
    if (GetAsyncKeyState(VK_TAB) & 0x0001)
    {
        is_debug_mode = !is_debug_mode;
        immediate_context->RSSetState(is_debug_mode ? debug_rasterizer_state : rasterizer_state);
    }
    
    if (scene)
        scene->update(delta_time);
}

void FApplication::draw()
{    
    // present unbinds render target, so rebind and clear at start of each frame
    float background_colour[4] = { 0.025f, 0.025f, 0.025f, 1.0f };  
    immediate_context->OMSetRenderTargets(1, &render_target_view, depth_stencil_view);
    immediate_context->ClearRenderTargetView(render_target_view, background_colour);
    immediate_context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
   
    if (scene && scene->active_camera)
    {
        for (FObject* object : scene->all_objects)
            drawObject(object);
    }

    // present backbuffer to screen
    swap_chain->Present(0, 0);
}

void FApplication::drawObject(FObject* object)
{
    if (!object) return;
    if (object->getType() != FObjectType::MESH) return;
    FMesh* mesh_object = (FMesh*)object;
    FMeshData* mesh_data = mesh_object->getData();
    if (!mesh_data) return;
    if (!mesh_data->index_buffer_ptr || !mesh_data->vertex_buffer_ptr) return;

    // store relevant data into the constant buffer for the shader to access
    XMFLOAT4X4 object_matrix = object->getTransform();
    XMFLOAT4X4 view_matrix = scene->active_camera->getTransform();
    XMFLOAT4X4 projection_matrix = scene->active_camera->getProjectionMatrix();
    XMVECTOR throwaway;
    constant_buffer_data.world = XMMatrixTranspose(XMLoadFloat4x4(&object_matrix));
    constant_buffer_data.view = XMMatrixTranspose(XMMatrixInverse(&throwaway, XMLoadFloat4x4(&view_matrix)));
    constant_buffer_data.projection = XMMatrixTranspose(XMLoadFloat4x4(&projection_matrix));

    // write constant buffer data onto GPU
    D3D11_MAPPED_SUBRESOURCE constant_buffer_resource;
    immediate_context->Map(constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constant_buffer_resource);
    memcpy(constant_buffer_resource.pData, &constant_buffer_data, sizeof(constant_buffer_data));
    immediate_context->Unmap(constant_buffer, 0);

    // set object variables and draw
    UINT stride = { sizeof(FVertex) };
    UINT offset = 0;
    immediate_context->IASetVertexBuffers(0, 1, &mesh_data->vertex_buffer_ptr, &stride, &offset);
    immediate_context->IASetIndexBuffer(mesh_data->index_buffer_ptr, DXGI_FORMAT_R16_UINT, 0);

    immediate_context->VSSetShader(vertex_shader, nullptr, 0);
    immediate_context->PSSetShader(pixel_shader, nullptr, 0);

    immediate_context->DrawIndexed(mesh_data->index_count, 0, 0);
}