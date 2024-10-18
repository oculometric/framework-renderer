#pragma once

#include <string>
#include <d3d11_4.h>

using namespace std;

class FShader
{
	friend class FResourceManager;
	friend class FApplication;
private:
public: // temporary. while i'm still initialising shaders manually
	inline FShader() { };

	// string text of shader
	string text;

	// TODO: complete this, come up with a nice way of configuring uniforms (ie constant buffer)
	// TODO: add control of pipeline state (wireframe, etc)

	// for rendering
	ID3D11VertexShader* vertex_shader_pointer = nullptr;
	ID3D11PixelShader* pixel_shader_pointer = nullptr;
};

class FMaterial
{
	friend class FResourceManager;
	friend class FApplication;
private:
public: // temporary. while i'm still initialising matierals manually
	inline FMaterial() { };

	// shader to be used
	FShader* shader;
	size_t constant_buffer_size;
	void* constant_buffer;

	FTexture* textures[4];

	// TODO: implement the whole shader thing
};