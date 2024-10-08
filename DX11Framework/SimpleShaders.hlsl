cbuffer ConstantBuffer : register(b0)
{
    float4x4 Projection;
    float4x4 View;
    float4x4 World;
}

struct VS_Out
{
    float4 position : SV_POSITION;
    float3 model_position : COLOR1;
    float3 world_position : COLOR2;
    float4 colour : COLOR;
    float3 normal : NORMAL;
};

VS_Out VS_main(float3 Position : POSITION, float4 Color : COLOR, float3 Normal : NORMAL)
{
    VS_Out output = (VS_Out)0;

    float4 Pos4 = float4(Position, 1.0f);
    output.model_position = Position;
    output.position = mul(Pos4, World);
    output.world_position = output.position;
    output.position = mul(output.position, View);
    output.position = mul(output.position, Projection);
    
    output.colour = Color;
    
    output.normal = normalize(mul(float4(Normal, 0.0f), World).xyz);
    
    return output;
}

float4 PS_main(VS_Out input) : SV_TARGET
{
    //float diffuse_light = clamp(-dot(float3(-1.0f, 0.2f, 0.5f), input.normal), 0.0f, 1.0f);
    float ambient_light = 0.02f;
    float3 colour = float3(0.8f, 0.8f, 0.8f);
    if (abs(floor(input.world_position.x * 5.0f)) % 2.0f == abs(floor(input.world_position.y * 5.0f)) % 2.0f)
        discard;
    return float4(input.world_position, 1.0f); //float4(input.normal, 1.0f); //input.color; //float4((colour * diffuse_light) + (colour * ambient_light), 1.0f); //input.color;

}