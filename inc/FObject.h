#pragma once

#include <DirectXMath.h>
#include <string>

#include "FTransform.h"

using namespace DirectX;

enum FObjectType
{
	EMPTY,
	CAMERA,
	MESH,
	LIGHT
};

// base object class implementing common properties across all objects
class FObject
{
public:
	std::string name = "Object";	// name for the object, to make debugging easier
	FTransform transform;			// transform component which handles position/rotation/scale of the object, and scene hierarchy (parenthood, childhood)

public:
	inline FObject() { };

	inline virtual FObjectType getType() { return FObjectType::EMPTY; }
};

// camera object type
class FCamera : public FObject
{
private:
	XMFLOAT4X4 projection_matrix = XMFLOAT4X4();

public:
	float aspect_ratio = 1.0f;		// X width over Y height
	float field_of_view = 90.0f;	// horizontal angle of view for the camera
	float near_clip = 0.01f;		// near clipping distance
	float far_clip = 100.0f;		// far clipping distance

public:
	inline FObjectType getType() { return FObjectType::CAMERA; }

	void updateProjectionMatrix();
	inline XMFLOAT4X4 getProjectionMatrix() const { return projection_matrix; }
};
