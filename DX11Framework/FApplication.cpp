#include "FApplication.h"

#include <string>
#include <queue>
#include <thread>
#include "DDSTextureLoader.h"

#include "FJsonParser.h"
#include "FScene.h"
#include "FResourceManager.h"

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
}

HRESULT FApplication::initialise(HINSTANCE hInstance, int nShowCmd)
{
    HRESULT hr = S_OK;

    FResourceManager::set(new FResourceManager(this));
    uniform_buffer = new float[4096];

    hr = createWindowHandle(hInstance, nShowCmd);
    if (FAILED(hr)) return E_FAIL;

    hr = createD3DDevice();
    if (FAILED(hr)) return E_FAIL;

    hr = createSwapChainAndFrameBuffer();
    if (FAILED(hr)) return E_FAIL;

    hr = initPipelineVariables();
    if (FAILED(hr)) return E_FAIL;

    return hr;
}

HRESULT FApplication::createWindowHandle(HINSTANCE hInstance, int nCmdShow)
{
    const wchar_t* window_name  = L"SpaceDemoProject";

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
    swap_chain_descriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
    swap_chain_descriptor.BufferCount = 2;
    swap_chain_descriptor.Scaling = DXGI_SCALING_STRETCH;
    swap_chain_descriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swap_chain_descriptor.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swap_chain_descriptor.Flags = 0;

    hr = dxgi_factory->CreateSwapChainForHwnd(device, window_handle, &swap_chain_descriptor, nullptr, nullptr, &swap_chain);
    if (FAILED(hr)) return hr;

    // grab a reference to the main render target texture
    hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&colour_buffer));
    
    // create a new texture with the same setup as the render target, which will be our intermediate for post-processing
    D3D11_TEXTURE2D_DESC colour_buffer_descriptor = { };
    colour_buffer->GetDesc(&colour_buffer_descriptor);
    hr = device->CreateTexture2D(&colour_buffer_descriptor, nullptr, &colour_buffer_intermediate);

    if (FAILED(hr)) return hr;

    // create a view around the render target
    D3D11_RENDER_TARGET_VIEW_DESC colour_buffer_view_descriptor = { };
    colour_buffer_view_descriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // sRGB render target enables hardware gamma correction
    colour_buffer_view_descriptor.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    hr = device->CreateRenderTargetView(colour_buffer, &colour_buffer_view_descriptor, &colour_buffer_view);

    // create a second, intermediate view around the intermediate render target
    colour_buffer_view_descriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    hr = device->CreateRenderTargetView(colour_buffer_intermediate, &colour_buffer_view_descriptor, &colour_buffer_intermediate_view);

    // create a shader resource view around the intermediate render target
    D3D11_SHADER_RESOURCE_VIEW_DESC colour_buffer_resource_descriptor = { };
    colour_buffer_resource_descriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    colour_buffer_resource_descriptor.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    colour_buffer_resource_descriptor.Texture2D.MipLevels = 1;
    colour_buffer_resource_descriptor.Texture2D.MostDetailedMip = 0;

    hr = device->CreateShaderResourceView(colour_buffer_intermediate, &colour_buffer_resource_descriptor, &colour_buffer_resource);

    if (FAILED(hr)) return hr;

    // create a new texture for the depth buffer
    D3D11_TEXTURE2D_DESC depth_buffer_descriptor = { };
    colour_buffer->GetDesc(&depth_buffer_descriptor);
    depth_buffer_descriptor.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depth_buffer_descriptor.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    device->CreateTexture2D(&depth_buffer_descriptor, nullptr, &depth_buffer);

    // create a view around the depth buffer
    D3D11_DEPTH_STENCIL_VIEW_DESC depth_buffer_view_descriptor = { };
    depth_buffer_view_descriptor.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_buffer_view_descriptor.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depth_buffer_view_descriptor.Texture2D.MipSlice = 0;
    device->CreateDepthStencilView(depth_buffer, &depth_buffer_view_descriptor, &depth_buffer_view);

    // create a shader resource view around the depth buffer
    colour_buffer_resource_descriptor.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

    hr = device->CreateShaderResourceView(depth_buffer, &colour_buffer_resource_descriptor, &depth_buffer_resource);

    if (FAILED(hr)) return hr;

    // create a normal buffer texture
    hr = device->CreateTexture2D(&colour_buffer_descriptor, nullptr, &normal_buffer);

    // create a render target view around that texture
    hr = device->CreateRenderTargetView(normal_buffer, &colour_buffer_view_descriptor, &normal_buffer_view);

    // create a shader resource view around the normal buffer
    colour_buffer_resource_descriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    hr = device->CreateShaderResourceView(normal_buffer, &colour_buffer_resource_descriptor, &normal_buffer_resource);

    return hr;
}

HRESULT FApplication::initPipelineVariables()
{
    HRESULT hr = S_OK;

    // set input assembler
    immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    // create viewport
    viewport = { 0.0f, 0.0f, (float)window_width, (float)window_height, 0.0f, 1.0f };
    immediate_context->RSSetViewports(1, &viewport);

    // create texture sampler
    D3D11_SAMPLER_DESC sampler_desc = { };
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.MaxLOD = 1;
    sampler_desc.MinLOD = 0;
    hr = device->CreateSamplerState(&sampler_desc, &bilinear_sampler_state);
    if (FAILED(hr)) { return hr; }

    immediate_context->PSSetSamplers(0, 1, &bilinear_sampler_state);

    // load default stuff
    hr = CreateDDSTextureFromFile(device, L"blank.dds", nullptr, &blank_texture);
    if (FAILED(hr)) { return hr; }

    FShader* shader = FResourceManager::get()->loadShader("SimpleShaders.hlsl", false, FCullMode::OFF);
    if (shader == nullptr) { return -1; }

    FJsonBlob material_blob("placeholder.fmat");
    FJsonElement mat_root = material_blob.getRoot();
    if (mat_root.type == JOBJECT && mat_root.o_val != nullptr)
    {
        FMaterialPreload mp;
        mat_root >> mp;
        placeholder_material = FResourceManager::get()->createMaterial("placeholder.mat", mp);
    }
    else
    {
        return -1;
    }

    postprocess_shader = FResourceManager::get()->loadShader("Postprocess.hlsl", false, FCullMode::OFF);
    FVertex quad_verts[] = 
    {
        FVertex{ XMFLOAT3(-1,-1,0), XMFLOAT4(), XMFLOAT3(), XMFLOAT2(-1,-1) },
        FVertex{ XMFLOAT3( 1,-1,0), XMFLOAT4(), XMFLOAT3(), XMFLOAT2( 1,-1) },
        FVertex{ XMFLOAT3( 1, 1,0), XMFLOAT4(), XMFLOAT3(), XMFLOAT2( 1, 1) },
        FVertex{ XMFLOAT3(-1, 1,0), XMFLOAT4(), XMFLOAT3(), XMFLOAT2(-1, 1) }
    };
    uint16_t quad_indices[] = 
    {
        0, 1, 2,
        0, 2, 3
    };
    D3D11_BUFFER_DESC vertex_buffer_descriptor = { };
    vertex_buffer_descriptor.ByteWidth = static_cast<UINT>(sizeof(FVertex) * 4);
    vertex_buffer_descriptor.Usage = D3D11_USAGE_IMMUTABLE;
    vertex_buffer_descriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vertex_subresource_data = { quad_verts };

    hr = device->CreateBuffer(&vertex_buffer_descriptor, &vertex_subresource_data, &quad_vertex_buffer);

    D3D11_BUFFER_DESC index_buffer_descriptor = { };
    index_buffer_descriptor.ByteWidth = static_cast<UINT>(sizeof(uint16_t) * 6);
    index_buffer_descriptor.Usage = D3D11_USAGE_IMMUTABLE;
    index_buffer_descriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA index_subresource_data = { quad_indices };

    hr = device->CreateBuffer(&index_buffer_descriptor, &index_subresource_data, &quad_index_buffer);

    if (FAILED(hr)) return hr;

    return S_OK;
}

bool FApplication::registerMesh(FMeshData* mesh_data)
{
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC vertex_buffer_descriptor = { };
    vertex_buffer_descriptor.ByteWidth = static_cast<UINT>(sizeof(FVertex) * mesh_data->vertices.size());
    vertex_buffer_descriptor.Usage = D3D11_USAGE_IMMUTABLE;
    vertex_buffer_descriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertex_subresource_data = { mesh_data->vertices.data() };

    hr = device->CreateBuffer(&vertex_buffer_descriptor, &vertex_subresource_data, &mesh_data->vertex_buffer_ptr);
    if (FAILED(hr)) return false;

    D3D11_BUFFER_DESC index_buffer_descriptor = { };
    index_buffer_descriptor.ByteWidth = static_cast<UINT>(sizeof(uint16_t) * mesh_data->indices.size());
    index_buffer_descriptor.Usage = D3D11_USAGE_IMMUTABLE;
    index_buffer_descriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA index_subresource_data = { mesh_data->indices.data() };

    hr = device->CreateBuffer(&index_buffer_descriptor, &index_subresource_data, &mesh_data->index_buffer_ptr);
    if (FAILED(hr)) return false;

    return true;
}

void FApplication::unregisterMesh(FMeshData* mesh_data)
{
    if (mesh_data == nullptr) return;

    if (mesh_data->vertex_buffer_ptr) { mesh_data->vertex_buffer_ptr->Release(); mesh_data->vertex_buffer_ptr = nullptr; }
    if (mesh_data->index_buffer_ptr) { mesh_data->index_buffer_ptr->Release(); mesh_data->index_buffer_ptr = nullptr; }
}

FTexture* FApplication::registerTexture(wstring path)
{
    FTexture* tex = new FTexture();
    
    HRESULT hr = S_OK;

    hr = CreateDDSTextureFromFile(device, path.c_str(), nullptr, &tex->buffer_ptr);
    return tex;
}

void FApplication::unregisterTexture(FTexture* texture)
{
    if (texture == nullptr) return;

    if (texture->buffer_ptr) { texture->buffer_ptr->Release(); texture->buffer_ptr = nullptr; }
}

bool FApplication::registerShader(FShader* shader, wstring path)
{
    HRESULT hr = S_OK;

    // configure rasterizer state
    D3D11_RASTERIZER_DESC rasterizer_descriptor = { };
    rasterizer_descriptor.FillMode = shader->draw_wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
    switch (shader->cull_mode)
    {
    case FCullMode::FRONT: rasterizer_descriptor.CullMode = D3D11_CULL_FRONT; break;
    case FCullMode::BACK:  rasterizer_descriptor.CullMode = D3D11_CULL_BACK; break;
    case FCullMode::OFF:   rasterizer_descriptor.CullMode = D3D11_CULL_NONE; break;
    }

    hr = device->CreateRasterizerState(&rasterizer_descriptor, &shader->rasterizer);
    if (FAILED(hr)) return false;

    // load shaders
    ID3DBlob* error_blob;

    DWORD shader_flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shader_flags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* vertex_shader_blob;
    hr = D3DCompileFromFile(path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_main", "vs_5_0", shader_flags, 0, &vertex_shader_blob, &error_blob);
    if (FAILED(hr))
    {
        MessageBoxA(window_handle, (char*)error_blob->GetBufferPointer(), nullptr, ERROR);
        error_blob->Release();
        return false;
    }
    hr = device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), nullptr, &shader->vertex_shader_pointer);

    if (FAILED(hr))
    {
        vertex_shader_blob->Release();
        error_blob->Release();
        return false;
    }

    ID3DBlob* pixel_shader_blob;
    hr = D3DCompileFromFile(path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_main", "ps_5_0", shader_flags, 0, &pixel_shader_blob, &error_blob);
    if (FAILED(hr))
    {
        MessageBoxA(window_handle, (char*)error_blob->GetBufferPointer(), nullptr, ERROR);
        error_blob->Release();
        return false;
    }
    hr = device->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), nullptr, &shader->pixel_shader_pointer);

    if (FAILED(hr))
    {
        shader->vertex_shader_pointer->Release();
        vertex_shader_blob->Release();
        pixel_shader_blob->Release();
        error_blob->Release();
        return false;
    }

    static D3D11_INPUT_ELEMENT_DESC input_element_descriptor[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hr = device->CreateInputLayout(input_element_descriptor, ARRAYSIZE(input_element_descriptor), vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), &shader->input_layout);
    if (FAILED(hr))
    {
        shader->vertex_shader_pointer->Release();
        shader->pixel_shader_pointer->Release();
        vertex_shader_blob->Release();
        pixel_shader_blob->Release();
        return false;
    }

    hr = D3DReflect(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&shader->reflector);
    if (FAILED(hr))
    {

        shader->vertex_shader_pointer->Release();
        shader->pixel_shader_pointer->Release();
        vertex_shader_blob->Release();
        pixel_shader_blob->Release();
        shader->input_layout->Release();
        return false;
    }
    
    D3D11_SHADER_BUFFER_DESC shader_buffer_descriptor = { };
    shader->reflector->GetConstantBufferByIndex(0)->GetDesc(&shader_buffer_descriptor);

    D3D11_BUFFER_DESC constant_buffer_descriptor = { };
    constant_buffer_descriptor.ByteWidth = max(shader_buffer_descriptor.Size, (UINT)16);
    constant_buffer_descriptor.Usage = D3D11_USAGE_DYNAMIC;
    constant_buffer_descriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_descriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = device->CreateBuffer(&constant_buffer_descriptor, nullptr, &shader->uniform_buffer);
    if (FAILED(hr))
    {
        shader->vertex_shader_pointer->Release();
        shader->pixel_shader_pointer->Release();
        vertex_shader_blob->Release();
        pixel_shader_blob->Release();
        shader->input_layout->Release();
        shader->reflector->Release();
        return false;
    }

    vertex_shader_blob->Release();
    pixel_shader_blob->Release();

    return true;
}

void FApplication::unregisterShader(FShader* shader)
{
    if (shader == nullptr) return;

    if (shader->input_layout) { shader->input_layout->Release(); shader->input_layout = nullptr; }
    if (shader->rasterizer) { shader->rasterizer->Release(); shader->rasterizer = nullptr; }
    if (shader->vertex_shader_pointer) { shader->vertex_shader_pointer->Release(); shader->vertex_shader_pointer = nullptr; }
    if (shader->pixel_shader_pointer) { shader->pixel_shader_pointer->Release(); shader->pixel_shader_pointer = nullptr; }
    if (shader->reflector) { shader->reflector->Release(); shader->reflector = nullptr; }
    if (shader->uniform_buffer) { shader->uniform_buffer->Release(); shader->uniform_buffer = nullptr; }
}

FApplication::~FApplication()
{
    delete FResourceManager::get();

    if (scene) delete scene;

    if (bilinear_sampler_state) bilinear_sampler_state->Release();
    if (blank_texture) blank_texture->Release();
    
    if (colour_buffer_resource) colour_buffer_resource->Release();
    if (colour_buffer_view) colour_buffer_view->Release();
    if (colour_buffer_intermediate_view) colour_buffer_intermediate_view->Release();
    if (colour_buffer) colour_buffer->Release();
    if (colour_buffer_intermediate) colour_buffer_intermediate->Release();

    if (depth_buffer_resource) depth_buffer_resource->Release();
    if (depth_buffer_view) depth_buffer_view->Release();
    if (depth_buffer) depth_buffer->Release();

    if (normal_buffer_resource) normal_buffer_resource->Release();
    if (normal_buffer_view) normal_buffer_view->Release();
    if (normal_buffer) normal_buffer->Release();

    if (quad_index_buffer) quad_index_buffer->Release();
    if (quad_vertex_buffer) quad_vertex_buffer->Release();

    delete uniform_buffer;

    if (swap_chain) swap_chain->Release();
    if (immediate_context) immediate_context->Release();
    if (device) device->Release();
    if (dxgi_device) dxgi_device->Release();
    if (dxgi_factory) dxgi_factory->Release();
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
        //immediate_context->RSSetState(is_debug_mode ? debug_rasterizer_state : rasterizer_state);
    }
    
    if (scene)
        scene->update(delta_time);
}

void FApplication::draw()
{    
    // present unbinds render target, so rebind and clear at start of each frame
    float background_colour[4] = { 0.025f, 0.025f, 0.025f, 1.0f };
    float zero[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    ID3D11RenderTargetView* targets[] = { colour_buffer_intermediate_view, normal_buffer_view };
    immediate_context->OMSetRenderTargets(2, targets, depth_buffer_view);
    immediate_context->ClearRenderTargetView(colour_buffer_intermediate_view, background_colour);
    immediate_context->ClearRenderTargetView(normal_buffer_view, zero);
    immediate_context->ClearDepthStencilView(depth_buffer_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    if (scene && scene->active_camera)
    {
        scene->active_camera->configuration.aspect_ratio = (float)window_width / (float)window_height;
        scene->active_camera->updateProjectionMatrix();
        for (FObject* object : scene->all_objects)
            drawObject(object);
    }

    performPostprocessing();

    // present backbuffer to screen
    swap_chain->Present(0, 0);
}

void FApplication::drawObject(FObject* object)
{
    // check if the object we have is valid and a mesh
    if (!object) return;
    if (object->getType() != FObjectType::MESH) return;
    FMesh* mesh_object = (FMesh*)object;
    FMeshData* mesh_data = mesh_object->getData();
    if (!mesh_data) return;
    if (!mesh_data->index_buffer_ptr || !mesh_data->vertex_buffer_ptr) return;

    // check if the object has a valid material
    FMaterial* material = mesh_object->getMaterial();
    if (material == nullptr)
        material = placeholder_material;
    FShader* shader = material->shader;

    // if this shader is not active, load all its properties
    if (shader != active_shader)
    {
        immediate_context->IASetInputLayout(shader->input_layout);
        immediate_context->VSSetShader(shader->vertex_shader_pointer, nullptr, 0);
        immediate_context->PSSetShader(shader->pixel_shader_pointer, nullptr, 0);
        immediate_context->RSSetState(shader->rasterizer);

        active_shader = shader;

        // make sure this uniform buffer is bound
        immediate_context->VSSetConstantBuffers(0, 1, &shader->uniform_buffer);
        immediate_context->PSSetConstantBuffers(0, 1, &shader->uniform_buffer);
    }

    ID3D11ShaderReflectionConstantBuffer* shader_reflection = shader->reflector->GetConstantBufferByIndex(0);
    D3D11_SHADER_BUFFER_DESC shader_buffer_descriptor = { };
    shader_reflection->GetDesc(&shader_buffer_descriptor);

    // store data to the constant buffer that is shared between all shaders
    XMFLOAT4X4 projection_matrix = scene->active_camera->getProjectionMatrix();
    XMFLOAT4X4 view_matrix = scene->active_camera->getTransform();
    XMFLOAT4X4 object_matrix = object->getTransform();
    ((XMMATRIX*)uniform_buffer)[0] = XMMatrixTranspose(XMLoadFloat4x4(&projection_matrix));
    ((XMMATRIX*)uniform_buffer)[1] = XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&view_matrix)));
    ((XMMATRIX*)uniform_buffer)[2] = XMMatrixTranspose(XMLoadFloat4x4(&view_matrix));
    ((XMMATRIX*)uniform_buffer)[3] = XMMatrixTranspose(XMLoadFloat4x4(&object_matrix));
    XMFLOAT4* light_params = (XMFLOAT4*)((uint8_t*)uniform_buffer + (sizeof(XMMATRIX) * 4));
    light_params[0] = XMFLOAT4(0.3f, 0.4f, -1.0f, 0.0f);    // direction
    light_params[1] = XMFLOAT4(0.8f, 0.7f, 0.6f, 1.0f);     // diffuse
    light_params[2] = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);     // specular
    light_params[3] = XMFLOAT4(0.05f, 0.04f, 0.02f, 1.0f);  // ambient

    // store the rest of the variables from the material
    for (size_t i = 8; i < shader_buffer_descriptor.Variables; i++)
    {
        ID3D11ShaderReflectionVariable* var = shader_reflection->GetVariableByIndex((UINT)i);
        D3D11_SHADER_VARIABLE_DESC var_descriptor = { };
        var->GetDesc(&var_descriptor);
        FMaterialParameter param = material->getParameter(var_descriptor.Name);
        void* start_ptr = ((uint8_t*)uniform_buffer) + var_descriptor.StartOffset;

        switch (param.type)
        {
        case FShaderUniformType::F1: ((FLOAT*)start_ptr)[0]      = param.f1; break;
        case FShaderUniformType::F3: ((XMFLOAT3*)start_ptr)[0]   = param.f3; break;
        case FShaderUniformType::F4: ((XMFLOAT4*)start_ptr)[0]   = param.f4; break;
        case FShaderUniformType::I1: ((INT*)start_ptr)[0]        = param.i1; break;
        case FShaderUniformType::I3: ((XMINT3*)start_ptr)[0]     = param.i3; break;
        case FShaderUniformType::M3: ((XMFLOAT3X3*)start_ptr)[0] = param.m3; break;
        case FShaderUniformType::M4: ((XMFLOAT4X4*)start_ptr)[0] = param.m4; break;
        case FShaderUniformType::INVALID: memset(start_ptr, (uint8_t)0, var_descriptor.Size); break;
        }
    }

    // write uniform buffer data onto GPU
    D3D11_MAPPED_SUBRESOURCE constant_buffer_resource;
    immediate_context->Map(shader->uniform_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constant_buffer_resource);
    memcpy(constant_buffer_resource.pData, uniform_buffer, shader_buffer_descriptor.Size);
    immediate_context->Unmap(shader->uniform_buffer, 0);

    for (size_t i = 0; i < MAX_TEXTURES; i++)
    {
        if (material->textures[i] == nullptr)
            immediate_context->PSSetShaderResources((UINT)i, 1, &blank_texture);
        else
            immediate_context->PSSetShaderResources((UINT)i, 1, &material->textures[i]->buffer_ptr);
    }

    // set object variables if this mesh is not currently active
    if (mesh_data != active_mesh)
    {
        UINT stride = { sizeof(FVertex) };
        UINT offset = 0;
        immediate_context->IASetVertexBuffers(0, 1, &mesh_data->vertex_buffer_ptr, &stride, &offset);
        immediate_context->IASetIndexBuffer(mesh_data->index_buffer_ptr, DXGI_FORMAT_R16_UINT, 0);
        active_mesh = mesh_data;;
    }

    // draw the object
    immediate_context->DrawIndexed(static_cast<UINT>(mesh_data->indices.size()), 0, 0);
}

void FApplication::performPostprocessing()
{
    ID3D11RenderTargetView* targets[] = { colour_buffer_view, nullptr };
    immediate_context->OMSetRenderTargets(2, targets, nullptr);

    // load input layout and shader
    immediate_context->IASetInputLayout(postprocess_shader->input_layout);
    immediate_context->VSSetShader(postprocess_shader->vertex_shader_pointer, nullptr, 0);
    immediate_context->PSSetShader(postprocess_shader->pixel_shader_pointer, nullptr, 0);
    immediate_context->RSSetState(postprocess_shader->rasterizer);

    // make sure this uniform buffer is bound
    immediate_context->VSSetConstantBuffers(0, 1, &postprocess_shader->uniform_buffer);
    immediate_context->PSSetConstantBuffers(0, 1, &postprocess_shader->uniform_buffer);

    active_shader = postprocess_shader;
    active_mesh = nullptr;

    // update uniform buffer contents
    XMFLOAT4X4 projection_matrix = scene->active_camera->getProjectionMatrix();
    XMFLOAT4X4 view_matrix = scene->active_camera->getTransform();
    ((XMMATRIX*)uniform_buffer)[0] = XMMatrixTranspose(XMLoadFloat4x4(&projection_matrix));
    ((XMMATRIX*)uniform_buffer)[1] = XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&view_matrix)));
    ((XMMATRIX*)uniform_buffer)[2] = XMMatrixTranspose(XMLoadFloat4x4(&view_matrix));

    // TODO: write other uniforms!

    ID3D11ShaderReflectionConstantBuffer* shader_reflection = postprocess_shader->reflector->GetConstantBufferByIndex(0);
    D3D11_SHADER_BUFFER_DESC shader_buffer_descriptor = { };
    shader_reflection->GetDesc(&shader_buffer_descriptor);

    // write uniform buffer data onto GPU
    D3D11_MAPPED_SUBRESOURCE constant_buffer_resource;
    immediate_context->Map(postprocess_shader->uniform_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constant_buffer_resource);
    memcpy(constant_buffer_resource.pData, uniform_buffer, shader_buffer_descriptor.Size);
    immediate_context->Unmap(postprocess_shader->uniform_buffer, 0);

    // bind the special spicy textures
    immediate_context->PSSetShaderResources(0, 1, &colour_buffer_resource);
    immediate_context->PSSetShaderResources(1, 1, &depth_buffer_resource);
    immediate_context->PSSetShaderResources(2, 1, &normal_buffer_resource);

    // bind vertex buffers
    UINT stride = { sizeof(FVertex) };
    UINT offset = 0;
    immediate_context->IASetVertexBuffers(0, 1, &quad_vertex_buffer, &stride, &offset);
    immediate_context->IASetIndexBuffer(quad_index_buffer, DXGI_FORMAT_R16_UINT, 0);

    // draw the quad
    immediate_context->DrawIndexed(6, 0, 0);

    // unbind the magic buffers
    ID3D11ShaderResourceView* tmp = nullptr;
    immediate_context->PSSetShaderResources(0, 1, &tmp);
    immediate_context->PSSetShaderResources(1, 1, &tmp);
    immediate_context->PSSetShaderResources(2, 1, &tmp);
}