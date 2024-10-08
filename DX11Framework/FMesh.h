#ifndef FMESH_H
#define FMESH_H

#include "FObject.h"
#include <d3d11_4.h>
#include <string>

struct FMeshData
{
	// per vertex data
	XMFLOAT3* position;
	XMFLOAT4* colour;

	// per face corner data
	uint16_t* indices;
	XMFLOAT3* normal;

	// counts
	uint16_t index_count;
	uint16_t vertex_count;

	// for rendering
	ID3D11Buffer* vertex_buffer_ptr;
	ID3D11Buffer* index_buffer_ptr;
};

struct FMaterial
{
	// TODO:
};

class FMesh : public FObject
{
private:
	FMeshData* mesh_data;
	FMaterial* material;

public:
	inline FObjectType getType() { return FObjectType::MESH; }
	
	static FMeshData loadMesh(string path);
	inline FMeshData* getData() { return mesh_data; }
	inline void setData(FMeshData* data) { mesh_data = data; }
	inline FMaterial* getMaterial() { return material; }
};

#endif