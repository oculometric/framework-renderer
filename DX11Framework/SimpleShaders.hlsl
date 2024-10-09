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
    float3 light_direction = normalize(float3(0.5f, 0.2f, -1.0f));
    float diffuse_light = -dot(light_direction, normalize(input.world_position));
    float ambient_light = 0.02f;
    float3 colour = float3(0.8f, 0.8f, 0.8f);
    
    float3 low_col = float3(87, 8, 3) / 255.0f;
    float3 mid_col = float3(145, 72, 16) / 255.0f;
    float3 lit_col = float3(255, 125, 7) / 255.0f;
    //if (abs(floor(input.world_position.x * 5.0f)) % 2.0f == abs(floor(input.world_position.y * 5.0f)) % 2.0f)
    //    discard;
    float exponent = 1.5f;
    float3 lit = lerp(lerp(mid_col, low_col, pow(clamp(-diffuse_light, 0.0f, 1.0f), exponent)), lit_col, pow(clamp(diffuse_light, 0.0f, 1.0f), exponent));
    return float4(lit * colour, 1.0f); //float4(input.normal, 1.0f); //input.color; //float4((colour * diffuse_light) + (colour * ambient_light), 1.0f); //input.color;

}