#pragma once

#include <d3d11_4.h>
#include <string>

#include "FComponent.h"
#include "FTexture.h"
#include "FMaterial.h"
#include "FBoundingBox.h"

// structure of a vertex, corresponds to the input layout used by all vertex shaders
struct FVertex
{
	XMFLOAT3 position = XMFLOAT3( 0.0f, 0.0f, 0.0f );
	XMFLOAT4 colour   = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	XMFLOAT3 normal   = XMFLOAT3( 0.0f, 0.0f, 0.0f );
	XMFLOAT2 uv       = XMFLOAT2( 0.0f, 0.0f );
	XMFLOAT3 tangent  = XMFLOAT3( 0.0f, 0.0f, 0.0f );
};

// structure for storing a mesh and its data
class FMesh
{
	friend class FMeshComponent;
	friend class FGraphicsEngine;

private:
	inline FMesh() { };

	// per vertex data
	std::vector<FVertex> vertices;

	// per face corner data
	std::vector<uint16_t> indices;

	// local-space bounding box
	FBoundingBox bounds;

	// GPU resources for rendering
	ID3D11Buffer* vertex_buffer_ptr = nullptr;
	ID3D11Buffer* index_buffer_ptr  = nullptr;
};

// component type which contains a mesh which will be drawn
class FMeshComponent : public FComponent
{
public:
	bool cast_shadow = true;		// toggles whether this mesh is included in the shadow mapping pass

private:
	FMesh* mesh_data = nullptr;	// mesh data to be used
	FMaterial* material  = nullptr;	// material data to be used

public:
	using FComponent::FComponent;

	inline FComponentType getType() const override { return FComponentType::MESH; }

	// custom OBJ loading function
	static FMesh* loadMesh(std::string path);
	
	// tests whether a ray intersects with an axis-aligned bounding box
	static bool intersectBoundingBox(const FBoundingBox& bb, XMFLOAT3 ray_origin, XMFLOAT3 ray_direction, float& tmin, float& tmax);

	inline FMesh* getData() { return mesh_data; }
	inline void setData(FMesh* data) { mesh_data = data; }
	inline FMaterial* getMaterial() { return material; }
	inline void setMaterial(FMaterial* mat) { material = mat; }
	inline FBoundingBox getMeshBounds() { return mesh_data == nullptr ? FBoundingBox{ } : mesh_data->bounds; }

	FBoundingBox getWorldSpaceBounds();
};