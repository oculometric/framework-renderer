#pragma once

#include <d3d11_4.h>
#include <string>

#include "FObject.h"
#include "FTexture.h"
#include "FMaterial.h"

struct FVertex
{
	XMFLOAT3 position = XMFLOAT3( 0.0f, 0.0f, 0.0f );
	XMFLOAT4 colour   = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	XMFLOAT3 normal   = XMFLOAT3( 0.0f, 0.0f, 0.0f );
	XMFLOAT2 uv       = XMFLOAT2( 0.0f, 0.0f );
	XMFLOAT3 tangent  = XMFLOAT3( 0.0f, 0.0f, 0.0f );
};

struct FBoundingBox
{
	XMFLOAT3 max_corner = XMFLOAT3( 0,0,0 );
	XMFLOAT3 min_corner = XMFLOAT3( 0,0,0 );
};

class FMeshData
{
	friend class FMesh;
	friend class FGraphicsEngine;
private:
	inline FMeshData() { };

	// per vertex data
	vector<FVertex> vertices;

	// per face corner data
	vector<uint16_t> indices;

	// local-space bounding box
	FBoundingBox bounds;

	// for rendering
	ID3D11Buffer* vertex_buffer_ptr = nullptr;
	ID3D11Buffer* index_buffer_ptr  = nullptr;
};

class FMesh : public FObject
{
private:
	FMeshData* mesh_data = nullptr;
	FMaterial* material  = nullptr;

public:
	inline FObjectType getType() { return FObjectType::MESH; }

	static FMeshData* loadMesh(string path);
	inline FMeshData* getData() { return mesh_data; }
	inline void setData(FMeshData* data) { mesh_data = data; }
	inline FMaterial* getMaterial() { return material; }
	inline void setMaterial(FMaterial* mat) { material = mat; }
	inline FBoundingBox getMeshBounds() { return mesh_data == nullptr ? FBoundingBox{ } : mesh_data->bounds; }

	FBoundingBox getWorldSpaceBounds();
};