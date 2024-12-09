#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class FObject;

enum FComponentType
{
	BLANK,
	CAMERA,
	MESH,
	LIGHT
};

class FComponent
{
	friend class FObject;
public:
	FComponent() = delete;
	FComponent(FObject* _owner) : owner(_owner) { }

	virtual inline FComponentType getType() { return FComponentType::BLANK; }
	FObject* getOwner() { return owner; }

protected:
	FObject* owner = nullptr;
};

// camera component
class FCamera : public FComponent
{
private:
	XMFLOAT4X4 projection_matrix = XMFLOAT4X4();

public:
	float aspect_ratio = 1.0f;		// X width over Y height
	float field_of_view = 90.0f;	// horizontal angle of view for the camera
	float near_clip = 0.01f;		// near clipping distance
	float far_clip = 100.0f;		// far clipping distance

public:
	using FComponent::FComponent;

	inline FComponentType getType() { return FComponentType::CAMERA; }

	void updateProjectionMatrix();
	inline XMFLOAT4X4 getProjectionMatrix() const { return projection_matrix; }
};
