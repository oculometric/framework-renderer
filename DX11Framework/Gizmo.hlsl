#include "Common.hlsl"

cbuffer ConstantBuffer : register(b0)
{
}

COMMON_CONSTANT_BUFFER;         // defines a second constant buffer in register b1 with the variable 'common'

struct Varyings
{
    float4 position         : SV_POSITION;  // clip space position
    float3 model_position   : COLOR1;       // position of the vertex in model space
    float3 world_position   : COLOR2;       // position of the vertex in world space
    float3 view_position    : COLOR3;       // position of the vertex in view space
    float4 colour           : COLOR0;
};

Varyings VS_main(float3 position : POSITION, float4 colour : COLOR, float3 normal : NORMAL, float2 uv : TEXCOORD0, float3 tangent : TANGENT0)
{
    Varyings output = (Varyings)0;
    
    output.model_position = position;
    output.world_position = mul(float4(output.model_position, 1), common.world_matrix).xyz;
    output.view_position = mul(float4(output.world_position, 1), common.view_matrix).xyz;
    output.position = mul(float4(output.view_position, 1), common.projection_matrix);
    
    output.colour = colour;
    
    return output;
}

struct ColourAndDepth
{
    float4 colour   : SV_TARGET0;
    float depth     : SV_DEPTH;
};

ColourAndDepth PS_main(Varyings input)
{
    ColourAndDepth cad = (ColourAndDepth)0;
    cad.colour = input.colour;
    cad.depth = 1.0f - (1.0f / (input.position.w + 1.0f));
    
    return cad;
}
