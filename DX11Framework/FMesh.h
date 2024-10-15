#ifndef FMESH_H
#define FMESH_H

#include "FObject.h"
#include <d3d11_4.h>
#include <string>

struct FVertex
{
	XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT4 colour = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT3 normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
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
	
	static FMeshData* loadMesh(string path);
	inline FMeshData* getData() { return mesh_data; }
	inline void setData(FMeshData* data) { mesh_data = data; }
	inline FMaterial* getMaterial() { return material; }
};

#endif