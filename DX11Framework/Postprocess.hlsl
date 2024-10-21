cbuffer ConstantBuffer : register(b0)
{
    float4x4 projection_matrix;
    float4x4 view_matrix;
    float4x4 view_matrix_inv;
    float4x4 world_matrix;
}

Texture2D screen : register(t0);
Texture2D depth  : register(t1);
Texture2D normal : register(t2);
SamplerState bilinear_sampler : register(s0);

struct Varyings
{
    float4 position  : SV_POSITION;
    float2 uv        : TEXCOORD0;
};

Varyings VS_main(float3 position : POSITION, float4 colour : COLOR, float3 normal : NORMAL, float2 uv : TEXCOORD0, float3 tangent : TANGENT0)
{
    Varyings output = (Varyings)0;
    output.position = float4(position, 1.0f);
    output.uv = uv;
    
    return output;
}

float4 PS_main(Varyings input) : SV_TARGET
{
    return float4(input.uv, 0.0f, 1.0f);

}