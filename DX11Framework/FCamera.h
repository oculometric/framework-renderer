#ifndef FCAMERA_H
#define FCAMERA_H

#include "FObject.h"

struct FCameraConfiguration
{
	float field_of_view = 90.0f;
};

class FCamera : public FObject
{
public:
	FCameraConfiguration configuration;

public:
	void lookAt(XMFLOAT3 eye, XMFLOAT3 target, XMFLOAT3 up);
};

#endif