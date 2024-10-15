class FApplication;

#ifndef FAPPLICATION_H
#define FAPPLICATION_H

#include <windows.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <chrono>

#include "FScene.h"
#include "FMesh.h"
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
	XMFLOAT4 material_specular;

	XMFLOAT4 light_direction[8];
	XMFLOAT4 light_diffuse[8];
	XMFLOAT4 light_specular[8];
	XMFLOAT4 light_ambient[8];
};

class FApplication
{
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

	ID3D11RasterizerState* rasterizer_state = nullptr;
	ID3D11RasterizerState* debug_rasterizer_state = nullptr;
	ID3D11VertexShader* vertex_shader = nullptr;
	ID3D11PixelShader* pixel_shader = nullptr;
	ID3D11InputLayout* input_layout = nullptr;
	ID3D11Buffer* constant_buffer = nullptr;
	ID3D11Texture2D* depth_stencil_buffer = nullptr;

	HWND window_handle;

	ConstantBuffer constant_buffer_data;

	//chrono::steady_clock::time_point last_frame_time;

public:
	FScene* scene;

private:
	void drawObject(FObject* object);

public:
	HRESULT initialise(HINSTANCE hInstance, int nCmdShow);
	HRESULT createWindowHandle(HINSTANCE hInstance, int nCmdShow);
	HRESULT createD3DDevice();
	HRESULT createSwapChainAndFrameBuffer();
	HRESULT initShadersAndInputLayout();
	HRESULT initPipelineVariables();

	void registerMesh(FMeshData* mesh_data);
	void unregisterMesh(FMeshData* mesh_data);

	~FApplication();
	void update();
	void draw();
};

#endif