#pragma once

#include <string>
#include <d3d11_4.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <map>
#include <vector>

#include "FTexture.h"

#define MAX_TEXTURES 8		// maximum number of texture slots which can be bound on a material

using namespace DirectX;

// data type for shader uniforms
enum FShaderUniformType
{
	F1, F3, F4,
	M3, M4,
	I1, I3,
	INVALID
};

// which side, if any, to cull of a mesh's triangles
enum FCullMode { OFF, BACK, FRONT };

// encapsulates shader parameters and keeps track of GPU resource parameters
class FShader
{
	friend class FResourceManager;
	friend class FGraphicsEngine;
	friend class FMaterial;

private:
	inline FShader() { };

	// rasterizer parameters
	bool draw_wireframe = false;
	FCullMode cull_mode = FCullMode::OFF;

	// GPU resources for rendering
	ID3D11VertexShader* vertex_shader_pointer = nullptr;
	ID3D11PixelShader* pixel_shader_pointer = nullptr;
	ID3D11RasterizerState* rasterizer = nullptr;
	ID3D11Buffer* uniform_buffer = nullptr;
	ID3D11InputLayout* input_layout = nullptr;
	ID3D11ShaderReflection* reflector = nullptr;
};

// represents a parameter which is applied to a material and corresponds to a uniform/constant value
struct FMaterialParameter
{
	FShaderUniformType type;

	union
	{
		FLOAT f1;
		XMFLOAT3 f3;
		XMFLOAT4 f4;
		XMFLOAT3X3 m3;
		XMFLOAT4X4 m4;
		INT i1;
		XMINT3 i3;
	};

	inline FMaterialParameter()               { type = FShaderUniformType::INVALID, m4 = XMFLOAT4X4();   }
	inline FMaterialParameter(FLOAT arg)      { type = FShaderUniformType::F1; f1 = arg; }
	inline FMaterialParameter(XMFLOAT3 arg)   { type = FShaderUniformType::F3; f3 = arg; }
	inline FMaterialParameter(XMFLOAT4 arg)   { type = FShaderUniformType::F4; f4 = arg; }
	inline FMaterialParameter(XMFLOAT3X3 arg) { type = FShaderUniformType::M3; m3 = arg; }
	inline FMaterialParameter(XMFLOAT4X4 arg) { type = FShaderUniformType::M4; m4 = arg; }
	inline FMaterialParameter(INT arg)        { type = FShaderUniformType::I1; i1 = arg; }
	inline FMaterialParameter(XMINT3 arg)     { type = FShaderUniformType::I3; i3 = arg; }
};

// encapsulates a material and its parameters, shader, and textures
class FMaterial
{
	friend class FResourceManager;
	friend class FGraphicsEngine;

private:
	inline FMaterial() { };

	// shader to be used
	FShader* shader = nullptr;

	// parameter map
	std::map<std::string, FMaterialParameter> parameters;

	// textures to be used
	FTexture* textures[MAX_TEXTURES] = { nullptr };

public:
	void setParameter(std::string name, FMaterialParameter param);
	FMaterialParameter getParameter(std::string name);
	void assignTexture(FTexture* tex, size_t index);
};