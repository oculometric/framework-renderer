cbuffer ConstantBuffer : register(b0)
{
    // these 8 fields MUST be the first 8
    float4x4 projection_matrix;
    float4x4 view_matrix;
    float4x4 view_matrix_inv;
    float4x4 world_matrix;
    
    float4 light_direction = float4(0.0f, 0.0f, -1.0f, 0.0f);
    float4 light_diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 light_specular = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 light_ambient = float4(0.05f, 0.05f, 0.05f, 1.0f);
    
    float time;
    float3 padding;
    
    float4 material_diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
}

Texture2D albedo : register(t0);
Texture2D normal : register(t1);
SamplerState bilinear_sampler : register(s0);

struct Varyings
{
    float4 position         : SV_POSITION;
    float3 model_position   : COLOR1;
    float3 world_position   : COLOR2;
    float3 view_position    : COLOR3;
    float4 colour           : COLOR;
    float3 normal           : NORMAL;
    float2 uv               : TEXCOORD0;
    float3 tangent          : TANGENT0;
    float3 bitangent        : BITANGENT0;
    
    float3x3 tbn            : MATRIX;
};

struct Fragment
{
    float4 colour           : SV_TARGET;
    float4 normal           : SV_TARGET1;
    float depth             : SV_DEPTH;
};

Varyings VS_main(float3 position : POSITION, float4 colour : COLOR, float3 normal : NORMAL, float2 uv : TEXCOORD0, float3 tangent : TANGENT0)
{
    Varyings output = (Varyings)0;
    
    float4 Pos4 = float4(position, 1.0f);
    output.model_position = position;
    output.position = mul(Pos4, world_matrix);
    output.world_position = output.position;
    output.position = mul(output.position, view_matrix);
    output.view_position = output.position;
    output.position = mul(output.position, projection_matrix);
    
    output.colour = colour;
    
    output.normal = normalize(mul(float4(normalize(normal), 0.0f), world_matrix).xyz);
    
    output.uv = uv * float2(1.0f, -1.0f);
    
    float3 bitangent = cross(normal, tangent);
    
    output.tangent = normalize(mul(float4(normalize(tangent), 0.0f), world_matrix).xyz);
    output.bitangent = normalize(cross(output.normal, output.tangent));
    
    output.tbn = float3x3(normalize(tangent), normalize(bitangent), normalize(normal));
    
    return output;
}

Fragment PS_main(Varyings input)
{
    // TODO: implement non-directional lights
    float3 view_dir = normalize(mul(float4(normalize(input.view_position.xyz), 0.0f), view_matrix_inv).xyz);
    float3 overall_colour = float3(0.0f, 0.0f, 0.0f);
    
    float3 surface_colour = material_diffuse.rgb * albedo.Sample(bilinear_sampler, input.uv).rgb;
    float3 surface_normal = (normal.Sample(bilinear_sampler, input.uv).xyz * 2.0f) - 1.0f;
    
    float3 true_normal = normalize(input.normal);
    if (length(surface_normal) <= 1.5f)
    {
        float3x3 tangent_matrix = input.tbn;
        // the fix was.... drum roll.......... switching these \/ two around. fucking kill me.
        true_normal = lerp(true_normal, mul(surface_normal, tangent_matrix), 1.0f);
    }
    else
    {
        true_normal = mul(float3(0,0,1), input.tbn);
    }
    
    //for (uint i = 0; i < 8; i++)
    //{
        float3 light_dir = normalize(light_direction.xyz);
    
        float dot_norm = dot(-light_dir, true_normal);
        
        float3 diffuse_light = saturate(dot_norm) * light_diffuse.rgb * surface_colour;
        float3 ambient_light = light_ambient.rgb * surface_colour;
    
        float3 specular_light = pow
        (
            saturate(dot
            (
                reflect(view_dir, true_normal),
                -light_dir
            )),
            material_diffuse.w
        ) * light_specular.rgb * surface_colour * (dot_norm > 0.0f);
        
        float3 colour = diffuse_light + ambient_light + specular_light;
        
        overall_colour += colour;
    //}
    
    Fragment output = (Fragment)0;
    output.colour = float4(overall_colour, 1.0f);
    output.normal = float4(true_normal, 1.0f);
    output.depth = 1.0f - (1.0f / (input.position.w + 1.0f));
    
    return output;
}