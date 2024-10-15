cbuffer ConstantBuffer : register(b0)
{
    float4x4 projection_matrix;
    float4x4 view_matrix;
    float4x4 view_matrix_inv;
    float4x4 world_matrix;
    
    float4 material_diffuse;
    float4 material_specular;
    
    float4 light_direction[8];
    float4 light_diffuse[8];
    float4 light_specular[8];
    float4 light_ambient[8];
}

struct Varyings
{
    float4 position : SV_POSITION;
    float3 model_position : COLOR1;
    float3 world_position : COLOR2;
    float3 view_position : COLOR3;
    float4 colour : COLOR;
    float3 normal : NORMAL;
};

Varyings VS_main(float3 Position : POSITION, float4 Color : COLOR, float3 Normal : NORMAL)
{
    Varyings output = (Varyings)0;
    
    float4 Pos4 = float4(Position, 1.0f);
    output.model_position = Position;
    output.position = mul(Pos4, world_matrix);
    output.world_position = output.position;
    output.position = mul(output.position, view_matrix);
    output.view_position = output.position;
    output.position = mul(output.position, projection_matrix);
    
    output.colour = Color;
    
    output.normal = normalize(mul(float4(Normal, 0.0f), world_matrix).xyz);
    
    return output;
}

float4 PS_main(Varyings input) : SV_TARGET
{
    // TODO: implement non-directional lights
    float3 view_dir = normalize(mul(float4(normalize(input.view_position.xyz), 0.0f), view_matrix_inv).xyz);
    float3 overall_colour = float3(0.0f, 0.0f, 0.0f);
    
    for (uint i = 0; i < 8; i++)
    {
        float3 light_dir = normalize(light_direction[i].xyz);
    
        float3 diffuse_light = saturate(dot(-light_dir, input.normal)) * light_diffuse[i].xyz * material_diffuse.xyz;
        float3 ambient_light = light_ambient[i].xyz;
    
        float3 specular_light = pow
        (
            saturate(dot
            (
                reflect(view_dir, input.normal),
                -light_dir
            )),
            material_specular.w
        ) * light_specular[i].xyz * material_specular.xyz;
        
        float3 colour = diffuse_light + ambient_light + specular_light;
        
        overall_colour += colour;
    }
    
    //float3 low_col = float3(87, 8, 3) / 255.0f;
    //float3 mid_col = float3(145, 72, 16) / 255.0f;
    //float3 lit_col = float3(255, 125, 7) / 255.0f;
    //if (abs(floor(input.world_position.x * 5.0f)) % 2.0f == abs(floor(input.world_position.y * 5.0f)) % 2.0f)
    //    discard;
    //float exponent = 1.5f;
    //float3 lit = lerp(lerp(mid_col, low_col, pow(clamp(-diffuse_light, 0.0f, 1.0f), exponent)), lit_col, pow(clamp(diffuse_light, 0.0f, 1.0f), exponent));
    //return float4(lit * colour, 1.0f); //float4(input.normal, 1.0f); //input.color; //float4((colour * diffuse_light) + (colour * ambient_light), 1.0f); //input.color;
    return float4(overall_colour, 1.0f);
}