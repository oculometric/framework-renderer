cbuffer ShadowMapConstantData : register(b0)
{
    float4x4 projection_matrix;
    float4x4 view_matrix;
    float4x4 world_matrix;
};

struct Varyings
{
    float4 position         : SV_POSITION;  // clip space position
};

Varyings VS_main(float3 position : POSITION, float4 colour : COLOR, float3 normal : NORMAL, float2 uv : TEXCOORD0, float3 tangent : TANGENT0)
{
    Varyings output = (Varyings)0;
    
    output.position = mul(mul(mul(float4(position, 1), world_matrix), view_matrix), projection_matrix);
    
    return output;
}

void PS_main(Varyings input) { }