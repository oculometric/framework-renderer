#pragma once

#include <d3d11_4.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#include "FApplication.h"
#include "FScene.h"
#include "FMaterial.h"
#include "FMesh.h"

using namespace DirectX;

#define NUM_LIGHTS 8
// this mirrors the Light struct defined in Light.hlsl
struct FLightData
{
	XMFLOAT3 colour;            // colour of the light
	FLOAT strength;             // brightness multiplier

	XMFLOAT4 light_direction;   // direction the light is pointing in. W component being 1 indiciates that the light is positional, 0 indicates directional

	XMFLOAT3 light_position;    // position of the light, only relevant for positional lights
	FLOAT angle;				// angle of the light in sin(degrees). angle = 0 will disable the light, angle = 1 will illuminate in a hemisphere, angle = -1 makes it a point light (as opposed to a spot light)
};

// this mirrors the CommonConstants struct defined in Common.hlsl
struct FCommonConstantData
{
	XMMATRIX projection_matrix; // takes vertices from view to clip space
	XMMATRIX view_matrix;       // takes vertices from world to view space
	XMMATRIX view_matrix_inv;   // takes vertices from view to world space
	XMMATRIX world_matrix;      // takes vertices from model to world space

	XMFLOAT4 light_ambient;     // ambient light colour
	FLightData lights[NUM_LIGHTS];   // array of lights affecting this object

	FLOAT time;                 // current world time in seconds
	XMFLOAT3 _;                 // padding
};

class FGraphicsEngine
{
	friend class FResourceManager;
	friend class FApplication;
private:
	FApplication* application = nullptr;

	D3D11_VIEWPORT viewport;
	IDXGISwapChain1* swap_chain = nullptr;

	ID3D11SamplerState* bilinear_sampler_state = nullptr;
	ID3D11ShaderResourceView* blank_texture = nullptr;

	ID3D11Texture2D* colour_buffer = nullptr;
	ID3D11Texture2D* colour_buffer_intermediate = nullptr;
	ID3D11RenderTargetView* colour_buffer_view = nullptr;
	ID3D11RenderTargetView* colour_buffer_intermediate_view = nullptr;
	ID3D11ShaderResourceView* colour_buffer_resource = nullptr;

	ID3D11Texture2D* depth_buffer = nullptr;
	ID3D11DepthStencilView* depth_buffer_view = nullptr;
	ID3D11ShaderResourceView* depth_buffer_resource = nullptr;

	ID3D11Texture2D* normal_buffer = nullptr;
	ID3D11RenderTargetView* normal_buffer_view = nullptr;
	ID3D11ShaderResourceView* normal_buffer_resource = nullptr;

	FShader* postprocess_shader = nullptr;
	ID3D11Buffer* quad_vertex_buffer = nullptr;
	ID3D11Buffer* quad_index_buffer = nullptr;
	ID3D11ShaderResourceView* skybox_texture = nullptr;

	FMaterial* placeholder_material = nullptr;
	FShader* active_shader = nullptr;
	FMeshData* active_mesh = nullptr;
	void* uniform_buffer_data = nullptr;
	FCommonConstantData* common_buffer_data = nullptr;
	ID3D11Buffer* common_buffer = nullptr;

private:
	FGraphicsEngine(FApplication* owner);

	HRESULT initialise();
	HRESULT createSwapChainAndFrameBuffer();
	HRESULT initPipelineVariables();
	HRESULT loadDefaultResources();

	void drawObject(FMesh* object);
	void performPostprocessing();

	bool registerMesh(FMeshData* mesh_data);
	void unregisterMesh(FMeshData* mesh_data);

	FTexture* registerTexture(wstring path);
	void unregisterTexture(FTexture* texture);

	bool registerShader(FShader* shader, wstring path);
	void unregisterShader(FShader* shader);

	inline ID3D11DeviceContext* getContext() { return application->getContext(); }
	inline ID3D11Device* getDevice() { return application->getDevice(); }
	inline IDXGIFactory2* getFactory() { return application->getFactory(); }
	inline FScene* getScene() { return application->scene; }
	inline float getTime() { return application->getTime(); }
	inline float getAspectRatio() { return getWidth() / getHeight(); }
	inline float getWidth() { return application->getWidth(); }
	inline float getHeight() { return application->getHeight(); }
	inline HWND getWindow() { return application->getWindow(); }

	~FGraphicsEngine();
public:
	void draw();
};