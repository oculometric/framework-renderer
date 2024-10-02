#ifndef FCAMERA_H
#define FCAMERA_H

#include "FObject.h"

struct FCameraConfiguration
{
	float field_of_view = 90.0f;
	float aspect_ratio = 1.0f;
	float near_clip = 0.01f;
	float far_clip = 100.0f;
};

class FCamera : public FObject
{
private:
	XMFLOAT4X4 projection_matrix;

public:
	FCameraConfiguration configuration;

public:
	inline FObjectType getType() { return FObjectType::CAMERA; }

	void lookAt(XMFLOAT3 eye, XMFLOAT3 target, XMFLOAT3 up);
	void updateProjectionMatrix();
	inline XMFLOAT4X4 getProjectionMatrix() { return projection_matrix; }

};

#endif