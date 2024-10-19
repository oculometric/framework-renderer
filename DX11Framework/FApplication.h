#pragma once

#include <windows.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <chrono>

#include "FScene.h"
#include "FMesh.h"
#include "FTexture.h"
//#include <wrl.h>

using namespace DirectX;
//using Microsoft::WRL::ComPtr;

struct ConstantBuffer
{
	XMMATRIX projection;
	XMMATRIX view;
	XMMATRIX view_inv;
	XMMATRIX world;

	XMFLOAT4 material_diffuse;

	XMFLOAT4 light_direction[8];
	XMFLOAT4 light_diffuse[8];
	XMFLOAT4 light_specular[8];
	XMFLOAT4 light_ambient[8];
};

class FApplication
{
	friend class FResourceManager;
private:
	int window_width = 1280;
	int window_height = 768;

	ID3D11DeviceContext* immediate_context = nullptr;
	ID3D11Device* device;
	IDXGIDevice* dxgi_device = nullptr;
	IDXGIFactory2* dxgi_factory = nullptr;
	ID3D11RenderTargetView* render_target_view = nullptr;
	ID3D11DepthStencilView* depth_stencil_view = nullptr;
	IDXGISwapChain1* swap_chain = nullptr;
	D3D11_VIEWPORT viewport;

	ID3D11SamplerState* bilinear_sampler_state = nullptr;
	ID3D11Texture2D* depth_stencil_buffer = nullptr;
	ID3D11ShaderResourceView* blank_texture = nullptr;

	HWND window_handle;

	FMaterial* demo_material = nullptr;
	FShader* active_shader = nullptr;
	FMeshData* active_mesh = nullptr;
	void* uniform_buffer = nullptr;

public:
	FScene* scene;

private:
	void drawObject(FObject* object);

	bool registerMesh(FMeshData* mesh_data);
	void unregisterMesh(FMeshData* mesh_data);

	FTexture* registerTexture(wstring path);
	void unregisterTexture(FTexture* texture);

	bool registerShader(FShader* shader, wstring path);
	void unregisterShader(FShader* shader);

public:
	HRESULT initialise(HINSTANCE hInstance, int nCmdShow);
	HRESULT createWindowHandle(HINSTANCE hInstance, int nCmdShow);
	HRESULT createD3DDevice();
	HRESULT createSwapChainAndFrameBuffer();
	HRESULT initPipelineVariables();

	inline bool isFocused() { return GetFocus() == window_handle; }

	~FApplication();
	void update();
	void draw();
};