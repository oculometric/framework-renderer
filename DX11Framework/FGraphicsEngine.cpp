#include "FGraphicsEngine.h"

#include "DDSTextureLoader.h"
#include "FResourceManager.h"
#include "FJsonParser.h"
#include "FDebug.h"
#include "Gizmo.h"

using namespace std;

FGraphicsEngine::FGraphicsEngine(FApplication* owner)
{
    application = owner;
}

HRESULT FGraphicsEngine::initialise()
{
    HRESULT hr;

    hr = createSwapChainAndFrameBuffer();
    if (FAILED(hr)) return hr;

    hr = initPipelineVariables();
    if (FAILED(hr)) return hr;

    hr = loadDefaultResources();
    if (FAILED(hr)) return hr;
    
    return hr;
}

HRESULT FGraphicsEngine::createSwapChainAndFrameBuffer()
{
    HRESULT hr = S_OK;

    DXGI_SWAP_CHAIN_DESC1 swap_chain_descriptor = { };
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

    hr = getFactory()->CreateSwapChainForHwnd(getDevice(), getWindow(), &swap_chain_descriptor, nullptr, nullptr, &swap_chain);
    if (FAILED(hr)) return hr;

    // grab a reference to the main render target texture
    hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&colour_buffer));

    // create a new texture with the same setup as the render target, which will be our intermediate for post-processing
    D3D11_TEXTURE2D_DESC colour_buffer_descriptor = { };
    colour_buffer->GetDesc(&colour_buffer_descriptor);
    hr = getDevice()->CreateTexture2D(&colour_buffer_descriptor, nullptr, &colour_buffer_intermediate);

    if (FAILED(hr)) return hr;

    // create a view around the render target
    D3D11_RENDER_TARGET_VIEW_DESC colour_buffer_view_descriptor = { };
    colour_buffer_view_descriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // sRGB render target enables hardware gamma correction
    colour_buffer_view_descriptor.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    if (colour_buffer == nullptr) return -1;
    hr = getDevice()->CreateRenderTargetView(colour_buffer, &colour_buffer_view_descriptor, &colour_buffer_view);

    // create a second, intermediate view around the intermediate render target
    colour_buffer_view_descriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    hr = getDevice()->CreateRenderTargetView(colour_buffer_intermediate, &colour_buffer_view_descriptor, &colour_buffer_intermediate_view);

    // create a shader resource view around the intermediate render target
    D3D11_SHADER_RESOURCE_VIEW_DESC colour_buffer_resource_descriptor = { };
    colour_buffer_resource_descriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    colour_buffer_resource_descriptor.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    colour_buffer_resource_descriptor.Texture2D.MipLevels = 1;
    colour_buffer_resource_descriptor.Texture2D.MostDetailedMip = 0;

    hr = getDevice()->CreateShaderResourceView(colour_buffer_intermediate, &colour_buffer_resource_descriptor, &colour_buffer_resource);

    if (FAILED(hr)) return hr;

    // create a new texture for the depth buffer
    D3D11_TEXTURE2D_DESC depth_buffer_descriptor = { };
    colour_buffer->GetDesc(&depth_buffer_descriptor);
    depth_buffer_descriptor.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depth_buffer_descriptor.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    getDevice()->CreateTexture2D(&depth_buffer_descriptor, nullptr, &depth_buffer);

    // create a view around the depth buffer
    D3D11_DEPTH_STENCIL_VIEW_DESC depth_buffer_view_descriptor = { };
    depth_buffer_view_descriptor.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_buffer_view_descriptor.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depth_buffer_view_descriptor.Texture2D.MipSlice = 0;
    if (depth_buffer == nullptr) return -1;
    getDevice()->CreateDepthStencilView(depth_buffer, &depth_buffer_view_descriptor, &depth_buffer_view);

    // create a shader resource view around the depth buffer
    colour_buffer_resource_descriptor.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

    hr = getDevice()->CreateShaderResourceView(depth_buffer, &colour_buffer_resource_descriptor, &depth_buffer_resource);

    if (FAILED(hr)) return hr;

    // create a normal buffer texture
    hr = getDevice()->CreateTexture2D(&colour_buffer_descriptor, nullptr, &normal_buffer);

    // create a render target view around that texture
    if (normal_buffer == nullptr) return -1;
    hr = getDevice()->CreateRenderTargetView(normal_buffer, &colour_buffer_view_descriptor, &normal_buffer_view);

    // create a shader resource view around the normal buffer
    colour_buffer_resource_descriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    hr = getDevice()->CreateShaderResourceView(normal_buffer, &colour_buffer_resource_descriptor, &normal_buffer_resource);

    return hr;
}

HRESULT FGraphicsEngine::initPipelineVariables()
{
    HRESULT hr = S_OK;

    // create viewport
    viewport = { 0.0f, 0.0f, getWidth(), getHeight(), 0.0f, 1.0f};
    getContext()->RSSetViewports(1, &viewport);

    // create texture sampler
    D3D11_SAMPLER_DESC sampler_desc = { };
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.MaxLOD = 1;
    sampler_desc.MinLOD = 0;
    hr = getDevice()->CreateSamplerState(&sampler_desc, &bilinear_sampler_state);
    if (FAILED(hr)) { return hr; }

    // create depth stencil states
    D3D11_DEPTH_STENCIL_DESC ds_desc = { };
    ds_desc.DepthEnable = true;
    ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    ds_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    ID3D11DepthStencilState* depth_stencil_state;
    getDevice()->CreateDepthStencilState(&ds_desc, &depth_stencil_state);
    getContext()->OMSetDepthStencilState(depth_stencil_state, 1);

    // create common constant buffer
    D3D11_BUFFER_DESC common_buffer_descriptor = { };
    common_buffer_descriptor.ByteWidth = sizeof(FCommonConstantData);
    common_buffer_descriptor.Usage = D3D11_USAGE_DYNAMIC;
    common_buffer_descriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    common_buffer_descriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = getDevice()->CreateBuffer(&common_buffer_descriptor, nullptr, &common_buffer);
    if (FAILED(hr)) { return hr; }

    common_buffer_data = new FCommonConstantData();
    uniform_buffer_data = new float[16384];

    return hr;
}

HRESULT FGraphicsEngine::loadDefaultResources()
{
    HRESULT hr = S_OK;

    // load blank texture
    hr = CreateDDSTextureFromFile(getDevice(), L"blank.dds", nullptr, &blank_texture);
    if (FAILED(hr)) { return hr; }

    FShader* shader = FResourceManager::get()->loadShader("PhysicalShader.hlsl", false, FCullMode::OFF);
    if (shader == nullptr) { return -1; }

    // load placeholder material
    FJsonBlob material_blob("placeholder.fmat");
    FJsonElement mat_root = material_blob.getRoot();
    if (mat_root.type == JOBJECT && mat_root.o_val != nullptr)
    {
        FMaterialPreload mp;
        mat_root >> mp;
        placeholder_material = FResourceManager::get()->createMaterial("placeholder.mat", mp);
    }
    else
        return E_FAIL;

    // load post processor
    postprocess_shader = FResourceManager::get()->loadShader("Postprocess.hlsl", false, FCullMode::OFF);
    if (postprocess_shader == nullptr) return E_FAIL;
    FVertex quad_verts[] =
    {
        FVertex{ XMFLOAT3(-1,-1,0), XMFLOAT4(), XMFLOAT3(), XMFLOAT2(-1,-1) },
        FVertex{ XMFLOAT3(1,-1,0), XMFLOAT4(), XMFLOAT3(), XMFLOAT2(1,-1) },
        FVertex{ XMFLOAT3(1, 1,0), XMFLOAT4(), XMFLOAT3(), XMFLOAT2(1, 1) },
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

    hr = getDevice()->CreateBuffer(&vertex_buffer_descriptor, &vertex_subresource_data, &quad_vertex_buffer);

    D3D11_BUFFER_DESC index_buffer_descriptor = { };
    index_buffer_descriptor.ByteWidth = static_cast<UINT>(sizeof(uint16_t) * 6);
    index_buffer_descriptor.Usage = D3D11_USAGE_IMMUTABLE;
    index_buffer_descriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA index_subresource_data = { quad_indices };

    hr = getDevice()->CreateBuffer(&index_buffer_descriptor, &index_subresource_data, &quad_index_buffer);

    if (FAILED(hr)) return hr;

    // create post process sampler
    D3D11_SAMPLER_DESC sampler_desc = { };
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    sampler_desc.MaxLOD = 1;
    sampler_desc.MinLOD = 0;
    hr = getDevice()->CreateSamplerState(&sampler_desc, &postprocess_sampler_state);
    if (FAILED(hr)) { return hr; }

    // load skybox
    hr = CreateDDSTextureFromFile(getDevice(), L"skybox.dds", nullptr, &skybox_texture);

    // load gizmo mesh
    vertex_buffer_descriptor = { };
    vertex_buffer_descriptor.ByteWidth = static_cast<UINT>(sizeof(FVertex) * gizmo_verts.size());
    vertex_buffer_descriptor.Usage = D3D11_USAGE_IMMUTABLE;
    vertex_buffer_descriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertex_subresource_data = { gizmo_verts.data() };

    hr = getDevice()->CreateBuffer(&vertex_buffer_descriptor, &vertex_subresource_data, &gizmo_vertex_buffer);

    index_buffer_descriptor = { };
    index_buffer_descriptor.ByteWidth = static_cast<UINT>(sizeof(uint16_t) * gizmo_inds.size());
    index_buffer_descriptor.Usage = D3D11_USAGE_IMMUTABLE;
    index_buffer_descriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;
    index_subresource_data = { gizmo_inds.data() };

    hr = getDevice()->CreateBuffer(&index_buffer_descriptor, &index_subresource_data, &gizmo_index_buffer);

    if (FAILED(hr)) return hr;

    vertex_buffer_descriptor = { };
    vertex_buffer_descriptor.ByteWidth = static_cast<UINT>(sizeof(FVertex) * box_verts.size());
    vertex_buffer_descriptor.Usage = D3D11_USAGE_IMMUTABLE;
    vertex_buffer_descriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertex_subresource_data = { box_verts.data() };

    hr = getDevice()->CreateBuffer(&vertex_buffer_descriptor, &vertex_subresource_data, &box_vertex_buffer);

    index_buffer_descriptor = { };
    index_buffer_descriptor.ByteWidth = static_cast<UINT>(sizeof(uint16_t) * box_inds.size());
    index_buffer_descriptor.Usage = D3D11_USAGE_IMMUTABLE;
    index_buffer_descriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;
    index_subresource_data = { box_inds.data() };

    hr = getDevice()->CreateBuffer(&index_buffer_descriptor, &index_subresource_data, &box_index_buffer);

    if (FAILED(hr)) return hr;

    // load gizmo shader
    gizmo_shader = FResourceManager::get()->loadShader("Gizmo.hlsl", true, FCullMode::OFF);
    if (gizmo_shader == nullptr) return E_FAIL;

    return S_OK;
}

bool FGraphicsEngine::registerMesh(FMeshData* mesh_data)
{
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC vertex_buffer_descriptor = { };
    vertex_buffer_descriptor.ByteWidth = static_cast<UINT>(sizeof(FVertex) * mesh_data->vertices.size());
    vertex_buffer_descriptor.Usage = D3D11_USAGE_IMMUTABLE;
    vertex_buffer_descriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertex_subresource_data = { mesh_data->vertices.data() };

    hr = getDevice()->CreateBuffer(&vertex_buffer_descriptor, &vertex_subresource_data, &mesh_data->vertex_buffer_ptr);
    if (FAILED(hr)) return false;

    D3D11_BUFFER_DESC index_buffer_descriptor = { };
    index_buffer_descriptor.ByteWidth = static_cast<UINT>(sizeof(uint16_t) * mesh_data->indices.size());
    index_buffer_descriptor.Usage = D3D11_USAGE_IMMUTABLE;
    index_buffer_descriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA index_subresource_data = { mesh_data->indices.data() };

    hr = getDevice()->CreateBuffer(&index_buffer_descriptor, &index_subresource_data, &mesh_data->index_buffer_ptr);
    if (FAILED(hr)) return false;

    return true;
}

void FGraphicsEngine::unregisterMesh(FMeshData* mesh_data)
{
    if (mesh_data == nullptr) return;

    if (mesh_data->vertex_buffer_ptr) { mesh_data->vertex_buffer_ptr->Release(); mesh_data->vertex_buffer_ptr = nullptr; }
    if (mesh_data->index_buffer_ptr) { mesh_data->index_buffer_ptr->Release(); mesh_data->index_buffer_ptr = nullptr; }
}

FTexture* FGraphicsEngine::registerTexture(wstring path)
{
    FTexture* tex = new FTexture();

    HRESULT hr = S_OK;

    hr = CreateDDSTextureFromFile(getDevice(), path.c_str(), nullptr, &tex->buffer_ptr);
    return tex;
}

void FGraphicsEngine::unregisterTexture(FTexture* texture)
{
    if (texture == nullptr) return;

    if (texture->buffer_ptr) { texture->buffer_ptr->Release(); texture->buffer_ptr = nullptr; }
}

bool FGraphicsEngine::registerShader(FShader* shader, wstring path)
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

    hr = getDevice()->CreateRasterizerState(&rasterizer_descriptor, &shader->rasterizer);
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
        FDebug::dialog(string((char*)error_blob->GetBufferPointer()));
        error_blob->Release();
        return false;
    }
    hr = getDevice()->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), nullptr, &shader->vertex_shader_pointer);

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
        FDebug::dialog(string((char*)error_blob->GetBufferPointer()));
        error_blob->Release();
        return false;
    }
    hr = getDevice()->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), nullptr, &shader->pixel_shader_pointer);

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

    hr = getDevice()->CreateInputLayout(input_element_descriptor, ARRAYSIZE(input_element_descriptor), vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), &shader->input_layout);
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

    if (FAILED(shader->reflector->GetConstantBufferByIndex(0)->GetDesc(nullptr)))
    {
        // if failed, use the pixel shader reflection instead
        shader->reflector->Release();
        hr = D3DReflect(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&shader->reflector);
        if (FAILED(hr))
        {
            shader->vertex_shader_pointer->Release();
            shader->pixel_shader_pointer->Release();
            vertex_shader_blob->Release();
            pixel_shader_blob->Release();
            shader->input_layout->Release();
            return false;
        }
    }

    D3D11_SHADER_BUFFER_DESC shader_buffer_descriptor = { };
    shader->reflector->GetConstantBufferByIndex(0)->GetDesc(&shader_buffer_descriptor);

    D3D11_BUFFER_DESC constant_buffer_descriptor = { };
    constant_buffer_descriptor.ByteWidth = max(shader_buffer_descriptor.Size, (UINT)sizeof(FCommonConstantData));
    constant_buffer_descriptor.Usage = D3D11_USAGE_DYNAMIC;
    constant_buffer_descriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_descriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = getDevice()->CreateBuffer(&constant_buffer_descriptor, nullptr, &shader->uniform_buffer);
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

void FGraphicsEngine::unregisterShader(FShader* shader)
{
    if (shader == nullptr) return;

    if (shader->input_layout) { shader->input_layout->Release(); shader->input_layout = nullptr; }
    if (shader->rasterizer) { shader->rasterizer->Release(); shader->rasterizer = nullptr; }
    if (shader->vertex_shader_pointer) { shader->vertex_shader_pointer->Release(); shader->vertex_shader_pointer = nullptr; }
    if (shader->pixel_shader_pointer) { shader->pixel_shader_pointer->Release(); shader->pixel_shader_pointer = nullptr; }
    if (shader->reflector) { shader->reflector->Release(); shader->reflector = nullptr; }
    if (shader->uniform_buffer) { shader->uniform_buffer->Release(); shader->uniform_buffer = nullptr; }
}

bool FGraphicsEngine::frustrumCull(XMFLOAT4X4 projection, XMFLOAT4X4 view_inv, FBoundingBox bounds)
{
    XMFLOAT3 mi = bounds.min_corner;
    XMFLOAT3 ma = bounds.max_corner;
    XMFLOAT4 nnn = XMFLOAT4(mi.x, mi.y, mi.z, 1);
    XMFLOAT4 ppp = XMFLOAT4(ma.x, ma.y, ma.z, 1);
    vector<XMFLOAT4> corners =
    {
        nnn,
        XMFLOAT4(ppp.x, nnn.y, nnn.z, 1),
        XMFLOAT4(nnn.x, ppp.y, nnn.z, 1),
        XMFLOAT4(ppp.x, ppp.y, nnn.z, 1),
        XMFLOAT4(nnn.x, nnn.y, ppp.z, 1),
        XMFLOAT4(ppp.x, nnn.y, ppp.z, 1),
        XMFLOAT4(nnn.x, ppp.y, ppp.z, 1),
        ppp
    };

    FBoundingBox world_bounds = FBoundingBox{ };
    XMMATRIX world_to_proj = XMMatrixIdentity() * XMMatrixInverse(nullptr, XMLoadFloat4x4(&view_inv)) * XMLoadFloat4x4(&projection);
    for (XMFLOAT4 v : corners)
    {
        XMFLOAT4 ws; XMStoreFloat4(&ws, XMVector4Transform(XMLoadFloat4(&v), world_to_proj));
        ws = XMFLOAT4(ws.x / ws.w, ws.y / ws.w, ws.z / ws.w, 1);

        if (ws.x <= 1 && ws.x >= -1
         && ws.y <= 1 && ws.y >= -1
         && ws.z <= 1 && ws.z >= -1)
            return true;
    }

    // TODO: improve this, sometimes it falseley culls things
    return false;
}

void FGraphicsEngine::sortForBatching(vector<FMesh*>& objects)
{
    if (objects.size() < 3) return;

    bool switched = true;
    while (switched)
    {
        switched = false;
        for (int i = 0; i < objects.size() - 1; i++)
        {
            FMesh* o1 = objects[i];
            FMesh* o2 = objects[i + 1];
            
            FMaterial* m1 = o1->getMaterial();
            FShader* s1 = m1 == nullptr ? nullptr : m1->shader;
            FMaterial* m2 = o2->getMaterial();
            FShader* s2 = m2 == nullptr ? nullptr : m2->shader;

            if (s1 > s2 || (s1 == s2 && m1 > m2))
            {
                objects[i + 1] = o1;
                objects[i] = o2;
                switched = true;
            }
        }
    }
}

FGraphicsEngine::~FGraphicsEngine()
{
    delete FResourceManager::get();

    if (bilinear_sampler_state) bilinear_sampler_state->Release();
    if (blank_texture) blank_texture->Release();
    if (common_buffer) common_buffer->Release();

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

    if (swap_chain) swap_chain->Release();

    delete uniform_buffer_data;
    delete common_buffer_data;
}

void FGraphicsEngine::draw()
{
    // present unbinds render target, so rebind and clear at start of each frame
    float clear[4] = { 0, 0, 0, 1 };
    float zero[4] = { 0, 0, 0, 1 };
    ID3D11RenderTargetView* targets[] = { colour_buffer_intermediate_view, normal_buffer_view };
    getContext()->OMSetRenderTargets(2, targets, depth_buffer_view);
    getContext()->ClearRenderTargetView(colour_buffer_intermediate_view, clear);
    getContext()->ClearRenderTargetView(normal_buffer_view, zero);
    getContext()->ClearDepthStencilView(depth_buffer_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    getContext()->PSSetSamplers(0, 1, &bilinear_sampler_state);

    if (getScene() && getScene()->active_camera)
    {
        // write out common constant buffer variables. none of these change per-object
        
        // store transformation data
        getScene()->active_camera->aspect_ratio = getAspectRatio();
        getScene()->active_camera->updateProjectionMatrix();
        XMFLOAT4X4 projection_matrix = getScene()->active_camera->getProjectionMatrix();
        XMFLOAT4X4 view_matrix_inv = getScene()->active_camera->getTransform();
        common_buffer_data->projection_matrix = XMMatrixTranspose(XMLoadFloat4x4(&projection_matrix));
        common_buffer_data->view_matrix = XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&view_matrix_inv)));
        common_buffer_data->view_matrix_inv = XMMatrixTranspose(XMLoadFloat4x4(&view_matrix_inv));

        // gather lights
        int i = 0;
        for (FLight* light : getScene()->all_lights)
        {
            light->convertToData(common_buffer_data->lights + i);
            i++;
            if (i >= NUM_LIGHTS) break;
        }
        for (i = i; i < NUM_LIGHTS; i++) common_buffer_data->lights[i] = FLightData{ };
        common_buffer_data->light_ambient = XMFLOAT4(0.05f, 0.04f, 0.02f, 1.0f); // TODO: ambient light, world config
        common_buffer_data->time = getTime();

        // ensure the common constant buffer is bound
        getContext()->VSSetConstantBuffers(1, 1, &common_buffer);
        getContext()->PSSetConstantBuffers(1, 1, &common_buffer);

        // extract all the objects we actually care about (no point sorting things we're going to discard)
        vector<FMesh*> batch;
        batch.reserve(getScene()->all_objects.size());
        for (FObject* obj : getScene()->all_objects)
            if (obj->getType() == FObjectType::MESH)
                if (frustrumCull(projection_matrix, view_matrix_inv, ((FMesh*)obj)->getWorldSpaceBounds()))
                    batch.push_back((FMesh*)obj);

        // sort the objects for batching, so we have fewer context/state switches
        sortForBatching(batch);

        // draw the objects
        for (FMesh* mo : batch)
            drawObject(mo);
    }

    performPostprocessing();

    drawGizmos();

    // present backbuffer to screen
    swap_chain->Present(0, 0);
}

void FGraphicsEngine::drawObject(FMesh* object)
{
    // check if the object we have is valid and a mesh
    FMesh* mesh_object = object;
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
        getContext()->IASetInputLayout(shader->input_layout);
        getContext()->VSSetShader(shader->vertex_shader_pointer, nullptr, 0);
        getContext()->PSSetShader(shader->pixel_shader_pointer, nullptr, 0);
        getContext()->RSSetState(shader->rasterizer);

        active_shader = shader;

        // make sure this uniform buffer is bound
        getContext()->VSSetConstantBuffers(0, 1, &shader->uniform_buffer);
        getContext()->PSSetConstantBuffers(0, 1, &shader->uniform_buffer);
    }

    // store data to the constant buffer that is shared between all shaders
    XMFLOAT4X4 object_matrix = object->getTransform();
    common_buffer_data->world_matrix = XMMatrixTranspose(XMLoadFloat4x4(&object_matrix));

    D3D11_MAPPED_SUBRESOURCE common_buffer_resource;
    getContext()->Map(common_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &common_buffer_resource);
    memcpy(common_buffer_resource.pData, (uint8_t*)common_buffer_data, sizeof(FCommonConstantData));
    getContext()->Unmap(common_buffer, 0);

    // store the rest of the variables from the material
    ID3D11ShaderReflectionConstantBuffer* shader_reflection = shader->reflector->GetConstantBufferByIndex(0);
    D3D11_SHADER_BUFFER_DESC shader_buffer_descriptor = { };
    shader_reflection->GetDesc(&shader_buffer_descriptor);

    for (size_t i = 0; i < shader_buffer_descriptor.Variables; i++)
    {
        ID3D11ShaderReflectionVariable* var = shader_reflection->GetVariableByIndex((UINT)i);
        D3D11_SHADER_VARIABLE_DESC var_descriptor = { };
        var->GetDesc(&var_descriptor);
        FMaterialParameter param = material->getParameter(var_descriptor.Name);
        void* start_ptr = ((uint8_t*)uniform_buffer_data) + var_descriptor.StartOffset;

        switch (param.type)
        {
        case FShaderUniformType::F1: ((FLOAT*)start_ptr)[0] = param.f1; break;
        case FShaderUniformType::F3: ((XMFLOAT3*)start_ptr)[0] = param.f3; break;
        case FShaderUniformType::F4: ((XMFLOAT4*)start_ptr)[0] = param.f4; break;
        case FShaderUniformType::I1: ((INT*)start_ptr)[0] = param.i1; break;
        case FShaderUniformType::I3: ((XMINT3*)start_ptr)[0] = param.i3; break;
        case FShaderUniformType::M3: ((XMFLOAT3X3*)start_ptr)[0] = param.m3; break;
        case FShaderUniformType::M4: ((XMFLOAT4X4*)start_ptr)[0] = param.m4; break;
        case FShaderUniformType::INVALID: memset(start_ptr, (uint8_t)0, var_descriptor.Size); break;
        }
    }

    // write uniform buffer data onto GPU
    D3D11_MAPPED_SUBRESOURCE constant_buffer_resource;
    getContext()->Map(shader->uniform_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constant_buffer_resource);
    memcpy(constant_buffer_resource.pData, uniform_buffer_data, max(shader_buffer_descriptor.Size, (UINT)sizeof(FCommonConstantData)));
    getContext()->Unmap(shader->uniform_buffer, 0);

    for (size_t i = 0; i < MAX_TEXTURES; i++)
    {
        if (material->textures[i] == nullptr)
            getContext()->PSSetShaderResources((UINT)i, 1, &blank_texture);
        else
            getContext()->PSSetShaderResources((UINT)i, 1, &material->textures[i]->buffer_ptr);
    }

    // set object variables if this mesh is not currently active
    if (mesh_data != active_mesh)
    {
        UINT stride = { sizeof(FVertex) };
        UINT offset = 0;
        getContext()->IASetVertexBuffers(0, 1, &mesh_data->vertex_buffer_ptr, &stride, &offset);
        getContext()->IASetIndexBuffer(mesh_data->index_buffer_ptr, DXGI_FORMAT_R16_UINT, 0);
        active_mesh = mesh_data;
    }

    // draw the object
    getContext()->DrawIndexed(static_cast<UINT>(mesh_data->indices.size()), 0, 0);
}

void FGraphicsEngine::performPostprocessing()
{
    ID3D11RenderTargetView* targets[] = { colour_buffer_view, nullptr };
    getContext()->OMSetRenderTargets(2, targets, nullptr);
    getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    getContext()->PSSetSamplers(0, 1, &postprocess_sampler_state);

    // load input layout and shader
    getContext()->IASetInputLayout(postprocess_shader->input_layout);
    getContext()->VSSetShader(postprocess_shader->vertex_shader_pointer, nullptr, 0);
    getContext()->PSSetShader(postprocess_shader->pixel_shader_pointer, nullptr, 0);
    getContext()->RSSetState(postprocess_shader->rasterizer);

    // make sure this uniform buffer is bound
    getContext()->VSSetConstantBuffers(0, 1, &postprocess_shader->uniform_buffer);
    getContext()->PSSetConstantBuffers(0, 1, &postprocess_shader->uniform_buffer);

    active_shader = postprocess_shader;
    active_mesh = nullptr;

    // update uniform buffer contents
    XMFLOAT4X4 projection_matrix = getScene()->active_camera->getProjectionMatrix();
    XMFLOAT4X4 view_matrix = getScene()->active_camera->getTransform();
    ((XMMATRIX*)uniform_buffer_data)[0] = XMMatrixTranspose(XMLoadFloat4x4(&projection_matrix));
    ((XMMATRIX*)uniform_buffer_data)[1] = XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&view_matrix)));
    ((XMMATRIX*)uniform_buffer_data)[2] = XMMatrixTranspose(XMLoadFloat4x4(&view_matrix));
    ((XMMATRIX*)uniform_buffer_data)[3] = XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&projection_matrix)));

    ID3D11ShaderReflectionConstantBuffer* shader_reflection = postprocess_shader->reflector->GetConstantBufferByIndex(0);
    D3D11_SHADER_BUFFER_DESC shader_buffer_descriptor = { };
    shader_reflection->GetDesc(&shader_buffer_descriptor);

    // write uniform buffer data onto GPU
    D3D11_MAPPED_SUBRESOURCE constant_buffer_resource;
    getContext()->Map(postprocess_shader->uniform_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constant_buffer_resource);
    memcpy(constant_buffer_resource.pData, uniform_buffer_data, shader_buffer_descriptor.Size);
    getContext()->Unmap(postprocess_shader->uniform_buffer, 0);

    // bind the special spicy textures
    getContext()->PSSetShaderResources(0, 1, &colour_buffer_resource);
    getContext()->PSSetShaderResources(1, 1, &normal_buffer_resource);
    getContext()->PSSetShaderResources(2, 1, &depth_buffer_resource);

    // bind skybox texture
    getContext()->PSSetShaderResources(3, 1, &skybox_texture);

    // bind vertex buffers
    UINT stride = { sizeof(FVertex) };
    UINT offset = 0;
    getContext()->IASetVertexBuffers(0, 1, &quad_vertex_buffer, &stride, &offset);
    getContext()->IASetIndexBuffer(quad_index_buffer, DXGI_FORMAT_R16_UINT, 0);

    // draw the quad
    getContext()->DrawIndexed(6, 0, 0);

    // unbind the magic buffers
    ID3D11ShaderResourceView* tmp = nullptr;
    getContext()->PSSetShaderResources(0, 1, &tmp);
    getContext()->PSSetShaderResources(1, 1, &tmp);
    getContext()->PSSetShaderResources(2, 1, &tmp);
}

void FGraphicsEngine::drawGizmos()
{
    // switch to gizmo draw mode
    ID3D11RenderTargetView* targets[] = { colour_buffer_view, nullptr };
    getContext()->OMSetRenderTargets(2, targets, depth_buffer_view);
    getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    getContext()->IASetInputLayout(gizmo_shader->input_layout);
    getContext()->VSSetShader(gizmo_shader->vertex_shader_pointer, nullptr, 0);
    getContext()->PSSetShader(gizmo_shader->pixel_shader_pointer, nullptr, 0);
    getContext()->RSSetState(gizmo_shader->rasterizer);

    // draw a set of axes at each object origin
    UINT stride = { sizeof(FVertex) };
    UINT offset = 0;
    getContext()->IASetVertexBuffers(0, 1, &gizmo_vertex_buffer, &stride, &offset);
    getContext()->IASetIndexBuffer(gizmo_index_buffer, DXGI_FORMAT_R16_UINT, 0);

    for (FObject* object : getScene()->all_objects)
    {
        if (object == getScene()->active_camera) continue;
        XMFLOAT4X4 object_matrix = object->getTransform();
        common_buffer_data->world_matrix = XMMatrixTranspose(XMLoadFloat4x4(&object_matrix));

        D3D11_MAPPED_SUBRESOURCE common_buffer_resource;
        getContext()->Map(common_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &common_buffer_resource);
        memcpy(common_buffer_resource.pData, (uint8_t*)common_buffer_data, sizeof(FCommonConstantData));
        getContext()->Unmap(common_buffer, 0);

        getContext()->DrawIndexed(static_cast<UINT>(gizmo_inds.size()), 0, 0);
    }

    if (getScene()->active_object == nullptr) return;
    if (getScene()->active_object->getType() != FObjectType::MESH) return;
    FMesh* mesh = (FMesh*)getScene()->active_object;

    // draw a bounding box for the selected object
    getContext()->IASetVertexBuffers(0, 1, &box_vertex_buffer, &stride, &offset);
    getContext()->IASetIndexBuffer(box_index_buffer, DXGI_FORMAT_R16_UINT, 0);

    FBoundingBox bounds = mesh->getWorldSpaceBounds();
    XMVECTOR max_c = XMLoadFloat3(&bounds.max_corner);
    XMVECTOR min_c = XMLoadFloat3(&bounds.min_corner);

    XMMATRIX world_matrix = XMMatrixIdentity() * XMMatrixScalingFromVector((max_c - min_c) / 2) * XMMatrixTranslationFromVector((max_c + min_c) / 2);
    common_buffer_data->world_matrix = XMMatrixTranspose(world_matrix);

    D3D11_MAPPED_SUBRESOURCE common_buffer_resource;
    getContext()->Map(common_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &common_buffer_resource);
    memcpy(common_buffer_resource.pData, (uint8_t*)common_buffer_data, sizeof(FCommonConstantData));
    getContext()->Unmap(common_buffer, 0);

    getContext()->DrawIndexed(static_cast<UINT>(box_inds.size()), 0, 0);
}
