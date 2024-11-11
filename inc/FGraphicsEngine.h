#pragma once

#include <d3d11_4.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#include "FApplication.h"
#include "FScene.h"
#include "FMaterial.h"
#include "FMesh.h"
#include "FLight.h"

using namespace DirectX;

// mirrors the CommonConstants struct defined in Common.hlsl
struct FCommonConstantData
{
	XMMATRIX projection_matrix; // takes vertices from view to clip space
	XMMATRIX view_matrix;       // takes vertices from world to view space
	XMMATRIX view_matrix_inv;   // takes vertices from view to world space
	XMMATRIX world_matrix;      // takes vertices from model to world space

	XMFLOAT4 light_ambient;     // ambient light colour
	FLightData lights[NUM_LIGHTS];   // array of lights affecting this object

	FLOAT time;                 // current world time in seconds
	XMFLOAT2 screen_size;		// size of screen in pixels
	FLOAT _;					// padding
};

// mirrors the contant buffer used by the shadow mapping shader
struct FShadowMapConstantData
{
	XMMATRIX projection_matrix;
	XMMATRIX view_matrix;
	XMMATRIX world_matrix;
};

// mirrors the constant buffer used by the post processing shader
struct FPostProcessConstantData
{
	XMMATRIX projection_matrix;
	XMMATRIX view_matrix;
	XMMATRIX view_matrix_inv;
	XMMATRIX projection_matrix_inv;
	XMFLOAT2 screen_size;
	XMFLOAT2 clipping_distances;
	XMFLOAT2 fog_start_end;
	XMFLOAT2 _;
	float fog_strength;
	XMFLOAT3 fog_colour;
	int output_mode;
};

#define NUM_SAMPLES 64

// mirrors the constant buffer used by the ambient occlusion shader
struct FAmbientOcclusionConstantData
{
	XMMATRIX projection_matrix;
	XMMATRIX view_matrix;
	XMMATRIX view_matrix_inv;
	XMMATRIX projection_matrix_inv;
	XMFLOAT2 screen_size;
	float radius;
	float _;
	XMFLOAT4 samples[NUM_SAMPLES];
};

// encapsulates all functionality which involves talking to the DirectX graphics API
class FGraphicsEngine
{
	friend class FResourceManager;
	friend class FApplication;

public:
	// view mode which is passed to the post-process shader for debug feedback
	enum FOutputMode
	{
		POST_PROCESS,
		SCENE_COLOUR,
		SCENE_NORMAL,
		SCENE_DEPTH,
		SHARPENED,
		AMBIENT_OCCLUSION
	};

	FOutputMode output_mode = POST_PROCESS;
	bool draw_gizmos = false;				// toggles drawing object axes and bounding box for debug

private:
	FApplication* application								= nullptr;	// reference to the owning application

	D3D11_VIEWPORT viewport;											// viewport for the game window
	IDXGISwapChain1* swap_chain								= nullptr;	// swap chain

	ID3D11SamplerState* nearest_sampler_state               = nullptr;	// sampler state which uses nearest/closest/point sampling
	ID3D11SamplerState* bilinear_sampler_state				= nullptr;	// sampler state which uses bilinear sampling
	ID3D11BlendState* alpha_blend_state						= nullptr;	// blend state used for all drawing, which uses simple additive blending based on source alpha
	ID3D11ShaderResourceView* blank_texture					= nullptr;	// contains a blank white 4x4 texture used for when a material has no texture bound to it
	ID3D11DepthStencilState* depth_stencil_state			= nullptr;	// depth stencil state used for all drawing

	ID3D11Texture2D* colour_buffer							= nullptr;	// texture used as the final render target/framebuffer, to which the post-processing quad is drawn
	ID3D11Texture2D* colour_buffer_intermediate				= nullptr; 	// texture used as intermediate framebuffer, to which objects in the scene are drawn
	ID3D11RenderTargetView* colour_buffer_view				= nullptr;	// render target view for the colour texture
	ID3D11RenderTargetView* colour_buffer_intermediate_view = nullptr;	// render target view for the intermediate colour texture
	ID3D11ShaderResourceView* colour_buffer_resource		= nullptr;	// shader resource view for the intermediate colour texture, for use with post-processing

	ID3D11Texture2D* depth_buffer							= nullptr;	// texture used as the intermediate depth render target, to which objects in the scene are drawn
	ID3D11DepthStencilView* depth_buffer_view				= nullptr;	// depth stencil view for the depth texture
	ID3D11ShaderResourceView* depth_buffer_resource			= nullptr;	// shader resource view for the depth texture, for use with post-processing

	ID3D11Texture2D* normal_buffer							= nullptr;	// texture used as the intermediate normal render target, to which objects in the scene are drawn
	ID3D11RenderTargetView* normal_buffer_view				= nullptr;	// render target view for the normal texture
	ID3D11ShaderResourceView* normal_buffer_resource		= nullptr;	// shader resource view for the normal texture, for use with post-processing

	ID3D11Texture2D* ao_buffer								= nullptr;	// texture used as the ambient occlusion render target, to which the AO shader draws
	ID3D11RenderTargetView* ao_buffer_view					= nullptr;	// render target view for the ambient occlusion texture
	ID3D11ShaderResourceView* ao_buffer_resource			= nullptr;	// shader resource view for the ambient occlusion texture

	FShader* postprocess_shader								= nullptr;	// shader which handles post-processing
	ID3D11Buffer* quad_vertex_buffer						= nullptr;	// vertex+index buffers for a simple quad, used for drawing the post-processing shader
	ID3D11Buffer* quad_index_buffer							= nullptr;
	ID3D11SamplerState* postprocess_sampler_state			= nullptr;	// sampler state used for sampling screen textures in the post-processing shader, prevents pixel wrapping
	ID3D11ShaderResourceView* skybox_texture				= nullptr;	// skybox texture, used with post-processing shader
	ID3D11ShaderResourceView* post_process_text_texture     = nullptr;	// text atlas texture, used with post-processing shader for ASCII effect
	FShader* ao_shader										= nullptr;	// shader applied to fullscreen quad for computing ambient occlusion pass
	FAmbientOcclusionConstantData* ao_buffer_data			= nullptr;	// constant buffer data to be passed to the AO shader

	FShader* gizmo_shader									= nullptr;	// shader which is used to draw gizmos
	ID3D11Buffer* gizmo_vertex_buffer						= nullptr;	// vertex+index buffers for a simple set of axes, coloured according to axis, used for gizmos
	ID3D11Buffer* gizmo_index_buffer						= nullptr;
	FShader* box_shader										= nullptr;	// shader which is used to draw bounding boxes
	ID3D11Buffer* box_vertex_buffer							= nullptr;	// vertex+index buffers for a cube, used for drawing bounding boxes
	ID3D11Buffer* box_index_buffer							= nullptr;

	FMeshData* active_mesh									= nullptr;	// currently bound vertex/index buffer data
	FShader* active_shader									= nullptr;	// currently bound shader
	FMaterial* active_material								= nullptr;	// currently bound material
	void* uniform_buffer_data								= nullptr;	// pointer to memory block used to stage constant buffer data for passing to the GPU
	FCommonConstantData* common_buffer_data					= nullptr;	// staging location for constant buffer which contains common data shared between shaders
	ID3D11Buffer* common_buffer								= nullptr;	// GPU buffer for constant buffer which contains common data shared between shaders

	ID3D11Texture2D* shadow_map_texture						= nullptr;	// texture array to which shadow maps are rendered
	ID3D11DepthStencilView* shadow_map_view[NUM_LIGHTS]		= { nullptr };	// array of depth stencil views corresponding to slices in the shadow map array, one for each light
	ID3D11ShaderResourceView* shadow_map_resource			= nullptr;	// shader resource view for the texture array
	FShadowMapConstantData* shadow_buffer_data				= nullptr;	// staging location for constant buffer containing data used by shadow mapping shader
	FShader* shadow_map_shader								= nullptr;	// shader used for rendering shadow maps, only writes to the depth buffer
	D3D11_VIEWPORT shadow_viewport;										// viewport used when rendering into the shadow map textures

	FMaterial* placeholder_material							= nullptr;	// material used when a mesh does not have a material assigned

	// used for keeping track of how many meshes, lights, and triangles have been drawn in the last frame
	int meshes;
	int lights;
	int tris;

private:
	FGraphicsEngine(FApplication* owner);

	HRESULT initialise();						// configures the graphics environment and engine
	HRESULT createSwapChainAndFrameBuffer();	// initialises the swapchain and framebuffers (and associated textures, resources, etc)
	HRESULT initPipelineVariables();			// initialises the rest of the rendering pipeline (depth stencil state, sampler states, etc)
	HRESULT loadDefaultResources();				// loads resources used by the engine (blank texture, post-processing shader, gizmo VB/IB, etc)

	void resizeRenderTargets();					// deletes the framebuffers and recreates them with a size to match that of the window

	// returns whether or not an object should be drawn, based on whether or not its bounding box intersects with the view frustrum
	bool frustrumCull(XMFLOAT4X4 projection, XMFLOAT4X4 view_inv, FBoundingBox bounds);
	// sorts objects according to their associated shader and returns only those which are meshes
	void sortForBatching(std::vector<FMesh*>& objects);
	void drawObject(FMesh* object);				// draws a mesh object to the intermediate framebuffer
	void performPostprocessing();				// renders the post-processing shader. this should only be called after all drawObject calls have been made
	void drawGizmos();							// draws debug gizmos. this should always be called after performPostprocessing
	void renderShadowMaps();					// renders the first NUM_LIGHTS lights in the scene to shadow map textures. this should be called before any drawObject calls are made

	bool registerMesh(FMeshData* mesh_data);	// uploads a mesh's index and vertex buffers to the GPU, returning true for success
	void unregisterMesh(FMeshData* mesh_data);	// unloads a mesh's index and vertex buffer data from the GPU

	FTexture* registerTexture(std::wstring path);	// loads a texture from a path, and returns it (or null if failure)
	void unregisterTexture(FTexture* texture);		// unloads a texture from the GPU

	bool registerShader(FShader* shader, std::wstring path);	// uploads a shader to the GPU, loading its code from the specified file path, and creating a rasteriser state according to the shader's configuration
	void unregisterShader(FShader* shader);						// unloads a shader from the GPU

	// getters for various fields
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
	void draw();			// draws the entire active scene, post-processing, and gizmos (if appropriate)
};