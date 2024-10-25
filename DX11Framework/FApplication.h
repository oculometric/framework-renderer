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

class FApplication
{
	friend class FResourceManager;
private:
	int window_width = 1280;
	int window_height = 768;

	ID3D11DeviceContext* immediate_context           = nullptr;
	ID3D11Device* device                             = nullptr;
	IDXGIDevice* dxgi_device                         = nullptr;
	IDXGIFactory2* dxgi_factory                      = nullptr;
	IDXGISwapChain1* swap_chain                      = nullptr;
	D3D11_VIEWPORT viewport;

	ID3D11SamplerState* bilinear_sampler_state       = nullptr;
	ID3D11ShaderResourceView* blank_texture          = nullptr;

	ID3D11Texture2D* colour_buffer							= nullptr;
	ID3D11Texture2D* colour_buffer_intermediate				= nullptr;
	ID3D11RenderTargetView* colour_buffer_view              = nullptr;
	ID3D11RenderTargetView* colour_buffer_intermediate_view = nullptr;
	ID3D11ShaderResourceView* colour_buffer_resource		= nullptr;

	ID3D11Texture2D* depth_buffer							= nullptr;
	ID3D11DepthStencilView* depth_buffer_view				= nullptr;
	ID3D11ShaderResourceView* depth_buffer_resource			= nullptr;

	ID3D11Texture2D* normal_buffer = nullptr;
	ID3D11RenderTargetView* normal_buffer_view = nullptr;
	ID3D11ShaderResourceView* normal_buffer_resource = nullptr;

	HWND window_handle;

	FShader* postprocess_shader      = nullptr;
	ID3D11Buffer* quad_vertex_buffer = nullptr;
	ID3D11Buffer* quad_index_buffer  = nullptr;
	ID3D11ShaderResourceView* skybox_texture = nullptr;

	FMaterial* placeholder_material  = nullptr;
	FShader* active_shader           = nullptr;
	FMeshData* active_mesh           = nullptr;
	void* uniform_buffer             = nullptr;
	float total_time = 0.0f;

public:
	FScene* scene;

private:
	void drawObject(FObject* object);
	void performPostprocessing();

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
	HRESULT loadDefaultResources();

	inline bool isFocused() { return GetFocus() == window_handle; }

	~FApplication();
	void update();
	void draw();
};