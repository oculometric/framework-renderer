#if !defined(COMMON_H)
#define COMMON_H

#define NUM_LIGHTS 16

#include "Light.hlsl"

struct Fragment
{
    float4 colour           : SV_TARGET0;   // colour buffer output
    float4 normal           : SV_TARGET1;   // normal buffer output
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
    float2 screen_size;         // size of screen in pixels
    float _;                    // padding
};

#define COMMON_CONSTANT_BUFFER cbuffer CommonConstantBuffer : register(b1) { CommonConstants common; }

float4 modelProjectionTransformation(float3 position, float4x4 world_matrix, out float3 world_position, float4x4 view_matrix, out float3 view_position, float4x4 projection_matrix)
{
    world_position = mul(float4(position, 1), world_matrix).xyz;
    view_position = mul(float4(world_position, 1), view_matrix).xyz;
    return mul(float4(view_position, 1), projection_matrix);
}

#endif