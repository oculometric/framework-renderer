#ifndef FMESH_H
#define FMESH_H

#include "FObject.h"

struct FMeshData
{
	// TODO:
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
};

#endif