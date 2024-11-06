#include "Common.hlsl"
#include "PBR.hlsl"

cbuffer ConstantBuffer : register(b0)
{
    float4 base_colour;         // base colour of the material. multiplied with all texture samples
    
    float normal_strength;      // strength of the normal mapping effect
}

COMMON_CONSTANT_BUFFER;         // defines a second constant buffer in register b1 with the variable 'common'

Texture2D      albedo       : register(t0); // contains surface colour information (alpha in A channel)
Texture2D      normal       : register(t1); // contains surface normal information
Texture2DArray shadow_map   : register(t8); // contains shadow atlas for the 8 lights // TODO: this

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

float random(float3 v)
{
    return frac(dot(v * float3(1245.3153, -122.1093, -160.4471), float3(-134.5313, -345.4627, 35.2370)));
}

Varyings VS_main(float3 position : POSITION, float4 colour : COLOR, float3 normal : NORMAL, float2 uv : TEXCOORD0, float3 tangent : TANGENT0)
{
    Varyings output = (Varyings)0;
    
    float3 perturbed_position = position;
    
    perturbed_position.z += (random(position) * 0.35f);
    
    output.position = modelProjectionTransformation(perturbed_position, common.world_matrix, output.world_position, common.view_matrix, output.view_position, common.projection_matrix);
    
    output.uv = uv * float2(1.0f, -1.0f);
    
    float3 transformed_normal = normalize(mul(float4(normalize(normal), 0.0f), common.world_matrix).xyz);
    output.normal = transformed_normal;
    float3 transformed_tangent = normalize(mul(float4(normalize(tangent), 0.0f), common.world_matrix).xyz);
    float3 bitangent = normalize(cross(transformed_normal, transformed_tangent));
    output.tbn = float3x3(transformed_tangent, bitangent, transformed_normal);
    
    return output;
}

Fragment PS_main(Varyings input)
{
    PBRSurface pbr_surface = (PBRSurface)0;
    pbr_surface.base_colour = base_colour;
    pbr_surface.specular_colour = float4(1, 1, 1, 1);
    pbr_surface.roughness_factor = 1.0f;
    pbr_surface.metallic_factor = 0.0f;
    pbr_surface.normal_strength = normal_strength;
    pbr_surface.emission_strength = 0.0f;
    
    PBRTextures pbr_textures;
    pbr_textures.albedo = albedo;
    pbr_textures.normal = normal;
    pbr_textures.shadow_map = shadow_map;
    pbr_textures.texture_sampler = bilinear_sampler;
    
    PBRConstants pbr_constants = (PBRConstants)0;
    pbr_constants.lights = common.lights;
    pbr_constants.light_ambient = common.light_ambient.rgb;
    pbr_constants.view_matrix_inv = common.view_matrix_inv;
    pbr_constants.using_triplanar = false;
    pbr_constants.triplanar_scale = 0.0f;
    
    PBRVaryings pbr_varyings = (PBRVaryings)0;
    pbr_varyings.position = input.position;
    pbr_varyings.view_position = input.view_position;
    pbr_varyings.world_position = input.world_position;
    pbr_varyings.normal = input.normal;
    pbr_varyings.uv = input.uv;
    pbr_varyings.tbn = input.tbn;
    
    float4 col;
    float3 norm;
    evaluateSurface(pbr_surface, pbr_textures, pbr_constants, pbr_varyings, col, norm);
    
    Fragment frag = (Fragment)0;
    frag.colour = col;
    frag.normal = float4(norm, 1.0f);
    
    return frag;
}
