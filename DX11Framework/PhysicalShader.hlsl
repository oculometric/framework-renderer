#define NUM_LIGHTS 8

struct Light
{
    float3 colour;              // colour of the light
    float strength;             // brightness multiplier
    
    float4 light_direction;     // direction the light is pointing in. W component being 1 indiciates that the light is positional, 0 indicates directional
    
    float3 light_position;      // position of the light, only relevant for positional lights
    float angle;                // angle of the light in sin(degrees). angle = 0 will disable the light, angle = 1 will illuminate in a hemisphere, angle = -1 makes it a point light (as opposed to a spot light)
    
    // TODO: maybe implement falloff (distance and angle) control for this?
};

struct Surface
{
    float4 base_colour;         // base colour of the material. multiplied with all texture samples
    float4 specular_colour;     // specular colour of the material
    
    float roughness_factor;     // surface roughness factor. blends between specular and diffuse lighting
    float metallic_factor;      // surface metallic factor. blends between using specular colour and albedo colour for specular highlight
    float normal_strength;      // strength of the normal mapping effect
    float emission_strength;    // strength of emission factor
};

cbuffer ConstantBuffer : register(b0)
{
    float4x4 projection_matrix; // takes vertices from view to clip space
    float4x4 view_matrix;       // takes vertices from world to view space
    float4x4 view_matrix_inv;   // takes vertices from view to world space
    float4x4 world_matrix;      // takes vertices from model to world space
    
    float4 light_ambient;       // ambient light colour
    Light lights[NUM_LIGHTS];   // array of lights affecting this object
    
    Surface pbr_surface;        // pbr surface parameters
    
    float time;                 // current world time in seconds
    float3 _;                   // padding
}

Texture2D      albedo       : register(t0); // contains surface colour information (alpha in A channel)
Texture2D      normal       : register(t1); // contains surface normal information
Texture2DArray shadow_map   : register(t2); // contains shadow atlas for the 8 lights

SamplerState bilinear_sampler : register(s0);

struct Varyings
{
    float4 position         : SV_POSITION;  // clip space position
    float3 model_position   : COLOR1;       // position of the vertex in model space
    float3 world_position   : COLOR2;       // position of the vertex in world space
    float3 view_position    : COLOR3;       // position of the vertex in view space
    float2 uv               : TEXCOORD0;    // uv coordinate for the vertex
    float3 normal           : NORMAL;       // normal in world space
    float3x3 tbn            : MATRIX;       // matrix where row0 is the tangent, row1 is the bitangent, row2 is the normal. all in world space
};

struct Fragment
{
    float4 colour           : SV_TARGET1;   // colour buffer output
    float4 normal           : SV_TARGET2;   // normal buffer output
    float depth             : SV_DEPTH;     // depth buffer output
};

Varyings VS_main(float3 position : POSITION, float4 colour : COLOR, float3 normal : NORMAL, float2 uv : TEXCOORD0, float3 tangent : TANGENT0)
{
    Varyings output = (Varyings)0;
    
    output.model_position = position;
    output.world_position = mul(float4(output.model_position, 1), world_matrix).xyz;
    output.view_position = mul(float4(output.world_position, 1), view_matrix).xyz;
    output.position = mul(float4(output.view_position, 1), projection_matrix);
    
    output.uv = uv * float2(1.0f, -1.0f);
    
    float3 transformed_normal = normalize(mul(float4(normalize(normal), 0.0f), world_matrix).xyz) * float3(-1, 1, 1);
    output.normal = transformed_normal;
    float3 transformed_tangent = normalize(mul(float4(normalize(tangent), 0.0f), world_matrix).xyz) * float3(-1, 1, 1);
    float3 bitangent = normalize(cross(transformed_normal, transformed_tangent)) * float3(-1, 1, 1);
    output.tbn = float3x3(transformed_tangent, bitangent, transformed_normal);
    
    return output;
}

// TODO: dithered alpha blending

Fragment evaluateSurface(Surface surface, Varyings input)
{
    // calculate direciton from the camera to the target fragment
    float3 view_dir = normalize(mul(float4(normalize(input.view_position.xyz), 0.0f), view_matrix_inv).xyz);
    
    // surface colour is the product of albedo (!!!) texture colour and base colour
    float3 surface_colour = albedo.Sample(bilinear_sampler, input.uv).rgb * surface.base_colour.rgb;
    // normal from the texture. possibly this is just blank
    float3 texture_normal = (normal.Sample(bilinear_sampler, input.uv).xyz * 2.0f) - 1.0f;
    // actual normal we're going to use for lighting
    float3 surface_normal = normalize(input.normal);
    if (length(texture_normal) <= 1.1f)
        surface_normal = lerp(surface_normal, mul(texture_normal, input.tbn), surface.normal_strength); // always make sure you have your multiplications the right way round, bucko!
    
    float3 overall_colour = lerp(float3(0,0,0), light_ambient.rgb * surface_colour, surface.roughness_factor);
    for (uint i = 0; i < NUM_LIGHTS; i++)
    {
        // TODO: implement non-directional lights
        
        Light light = lights[i];
        // direction of the light in world space
        float3 light_dir = normalize(light.light_direction.xyz);
        // light colour with strength applied
        float3 light_col = light.colour.rgb * light.strength;
        // dot product between the light direction and the surface normal
        float dir_dot_norm = dot(-light_dir, surface_normal);
        // diffuse component of the light's contribution
        float3 diffuse_light = saturate(dir_dot_norm) * light_col * surface_colour;
        // specular exponent. the higher the roughness, the more spread-out the specular highlight should be (and the less it should contribute)
        // TODO: make this physically accurate (energy-preserving)
        float specular_exp = 10.0f * (1.0f - surface.roughness_factor);
        // specular component of the light's contribution
        float specular_highlight = pow(saturate(dot(reflect(view_dir, surface_normal), -light_dir)), specular_exp) * (dir_dot_norm > 0.0f);
        float3 specular_colour = lerp(specular_colour.rgb, surface_colour, surface.metallic_factor);
        float3 specular_light = specular_highlight * light_col * surface_colour;
        
        // TODO: as above, energy preservation
        float3 colour = (diffuse_light * surface.roughness_factor) + (specular_light * (1.0f - surface.roughness_factor));
        
        overall_colour += colour;
    }
    
    // apply emission component
    overall_colour += surface.emission_strength * surface_colour;
    
    Fragment output = (Fragment)0;
    output.colour = float4(overall_colour, 1.0f);
    output.normal = float4(surface_normal, 1.0f);
    output.depth = 1.0f - (1.0f / (input.position.w + 1.0f));
    
    return output;
}

Fragment PS_main(Varyings input)
{
    return evaluateSurface(pbr_surface, input);
}
