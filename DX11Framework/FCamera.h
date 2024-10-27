#pragma once

#include "FObject.h"

class FCamera : public FObject
{
private:
	XMFLOAT4X4 projection_matrix = XMFLOAT4X4();

public:
	float aspect_ratio = 1.0f;
	float field_of_view = 90.0f;
	float near_clip = 0.01f;
	float far_clip = 100.0f;

public:
	inline FObjectType getType() { return FObjectType::CAMERA; }

	void lookAt(XMFLOAT3 eye, XMFLOAT3 target, XMFLOAT3 up);
	void updateProjectionMatrix();
	inline XMFLOAT4X4 getProjectionMatrix() const { return projection_matrix; }
};