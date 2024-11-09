#pragma once

#include "FObject.h"

#define NUM_LIGHTS 8		// number of lights which can render simultaneously
#define LIGHTMAP_SIZE 2048	// size of the shadow map texture for each light

// this mirrors the Light struct defined in Light.hlsl
struct FLightData
{
	XMFLOAT3 colour;            // colour of the light
	float strength;             // brightness multiplier

	XMFLOAT4 light_direction;   // direction the light is pointing in. W component being 1 indiciates that the light is positional, 0 indicates directional

	XMFLOAT3 light_position;    // position of the light, only relevant for positional lights
	float angle;				// angle of the light in sin(degrees). angle = 0 will disable the light, angle = 1 will illuminate in a hemisphere, angle = -1 makes it a point light (as opposed to a spot light)

	XMFLOAT4X4 matrix;
};

// an object type which behaves as a light
class FLight : public FObject
{
public:
	enum FLightType
	{
		DIRECTIONAL,
		POINT,
		SPOT
	};

public:
	FLightType type = FLightType::DIRECTIONAL;

	XMFLOAT3 colour = XMFLOAT3(1, 1, 1);
	float strength = 1.0f;

	// direction is only relevant for directional and spot lights
	// position is only relevant for point and spot lights
	// angle is only relevant for spot lights

	float angle = 45.0f; // in degrees

public:
	inline FObjectType getType() { return FObjectType::LIGHT; }
	
	void convertToData(FLightData* data);	// constructs a light data configuration from the light object
	XMFLOAT4X4 getProjectionMatrix();		// returns the projection matrix for the light, treating it as a camera
};
