#ifndef FMESH_H
#define FMESH_H

#include "FObject.h"
#include <d3d11_4.h>
#include <string>
#include "FTexture.h"

struct FVertex
{
	XMFLOAT3 position = XMFLOAT3( 0.0f, 0.0f, 0.0f );
	XMFLOAT4 colour   = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	XMFLOAT3 normal   = XMFLOAT3( 0.0f, 0.0f, 0.0f );
	XMFLOAT2 uv       = XMFLOAT2( 0.0f, 0.0f );
	XMFLOAT3 tangent  = XMFLOAT3( 0.0f, 0.0f, 0.0f );
};

struct FMeshData
{
	// per vertex data
	vector<FVertex> vertices;

	// per face corner data
	vector<uint16_t> indices;

	// for rendering
	ID3D11Buffer* vertex_buffer_ptr;
	ID3D11Buffer* index_buffer_ptr;
};

struct FShader
{
	// string text of shader
	string text;
	
	// TODO: complete this, come up with a nice way of configuring uniforms (ie constant buffer)
	// TODO: add control of pipeline state (wireframe, etc)

	// for rendering
	ID3D11VertexShader* vertex_shader_pointer;
	ID3D11PixelShader* pixel_shader_pointer;
};

struct FMaterial
{
	// shader to be used
	FShader* shader;
	size_t constant_buffer_size;
	void* constant_buffer;

	FTexture* textures[4];

	// TODO: implement the whole shader thing
};

class FMesh : public FObject
{
private:
	FMeshData* mesh_data;
	FMaterial* material;

public:
	inline FObjectType getType() { return FObjectType::MESH; }
	
	static FMeshData* loadMesh(string path);
	inline FMeshData* getData() { return mesh_data; }
	inline void setData(FMeshData* data) { mesh_data = data; }
	inline FMaterial* getMaterial() { return material; }
	inline void setMaterial(FMaterial* mat) { material = mat; }
};

#endif