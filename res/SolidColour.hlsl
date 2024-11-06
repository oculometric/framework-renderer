#include "Common.hlsl"

cbuffer ConstantBuffer : register(b0)
{
    float4 base_colour;
}

COMMON_CONSTANT_BUFFER;         // defines a second constant buffer in register b1 with the variable 'common'

struct Varyings
{
    float4 position         : SV_POSITION;  // clip space position
};

Varyings VS_main(float3 position : POSITION, float4 colour : COLOR, float3 normal : NORMAL, float2 uv : TEXCOORD0, float3 tangent : TANGENT0)
{
    Varyings output = (Varyings)0;
    
    float3 wp;
    float3 vp;
    
    output.position = modelProjectionTransformation(position, common.world_matrix, wp, common.view_matrix, vp, common.projection_matrix);
    
    return output;
}

Fragment PS_main(Varyings input)
{
    Fragment frag = (Fragment)0;
    frag.colour = base_colour;
    return frag;
}
