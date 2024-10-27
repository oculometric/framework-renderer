#if !defined(COMMON_H)
#define COMMON_H

#define NUM_LIGHTS 8

#include "Light.hlsl"

struct Fragment
{
    float4 colour           : SV_TARGET1;   // colour buffer output
    float4 normal           : SV_TARGET2;   // normal buffer output
    float depth             : SV_DEPTH;     // depth buffer output
};

struct CommonConstants
{
    float4x4 projection_matrix; // takes vertices from view to clip space
    float4x4 view_matrix;       // takes vertices from world to view space
    float4x4 view_matrix_inv;   // takes vertices from view to world space
    float4x4 world_matrix;      // takes vertices from model to world space
    
    float4 light_ambient;       // ambient light colour
    Light lights[NUM_LIGHTS];   // array of lights affecting this object
    
    float time;                 // current world time in seconds
    float3 _;                   // padding
};

#endif