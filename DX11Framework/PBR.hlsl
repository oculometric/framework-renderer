#if !defined(PBR_H)
#define PBR_H

#include "Light.hlsl"
#include "Common.hlsl"

struct PBRSurface
{
    float4 base_colour;         // base colour of the material. multiplied with all texture samples
    float4 specular_colour;     // specular colour of the material
    
    float roughness_factor;     // surface roughness factor. blends between specular and diffuse lighting
    float metallic_factor;      // surface metallic factor. blends between using specular colour and albedo colour for specular highlight
    float normal_strength;      // strength of the normal mapping effect
    float emission_strength;    // strength of emission factor
};

struct PBRConstants
{
    Light lights[NUM_LIGHTS];   // list of lights to be sampled
    float3 light_ambient;       // ambient light colour
    float4x4 view_matrix_inv;   // inverse view matrix
};

struct PBRVaryings
{
    float4 position;            // clip space position of the fragment
    float3 view_position;       // view space position of the fragment
    float3 normal;              // world space normal of the fragment
    float2 uv;                  // uv coordinate of the fragment
    float3x3 tbn;               // tangent-bitangent-normal matrix
};

struct PBRTextures
{
    Texture2D albedo;           // texture to be sampled for colour data
    Texture2D normal;           // texture to be sampled for normal data
    SamplerState texture_sampler;
};

// TODO: dithered alpha blending

void evaluateSurface(PBRSurface surface, PBRTextures textures, PBRConstants constants, PBRVaryings varyings, out float4 colour, out float3 normal, out float depth)
{
    // calculate direciton from the camera to the target fragment
    float3 view_dir = normalize(mul(float4(normalize(varyings.view_position), 1), constants.view_matrix_inv).xyz);
    
    // surface colour is the product of albedo (!!!) texture colour and base colour
    float3 surface_colour = textures.albedo.Sample(textures.texture_sampler, varyings.uv).rgb * surface.base_colour.rgb;
    // normal from the texture. possibly this is just blank
    float3 texture_normal = (textures.normal.Sample(textures.texture_sampler, varyings.uv).xyz * 2.0f) - 1.0f;
    // actual normal we're going to use for lighting
    float3 surface_normal = normalize(varyings.normal);
    if (length(texture_normal) <= 1.1f)
        surface_normal = lerp(surface_normal, mul(texture_normal, varyings.tbn), surface.normal_strength); // always make sure you have your multiplications the right way round, bucko!
    
    float3 overall_colour = lerp(float3(0,0,0), constants.light_ambient.rgb * surface_colour, surface.roughness_factor);
    for (uint i = 0; i < NUM_LIGHTS; i++)
    {
        // TODO: implement non-directional lights
        
        Light light = constants.lights[i];
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
        float specular_highlight = pow(saturate(dot(reflect(view_dir, surface_normal), light_dir)), specular_exp) * (dir_dot_norm > 0.0f);
        float3 specular_colour = lerp(specular_colour.rgb, surface_colour, surface.metallic_factor);
        float3 specular_light = specular_highlight * light_col * surface_colour;
        
        // TODO: as above, energy preservation
        float3 colour = (diffuse_light * surface.roughness_factor) + (specular_light * (1.0f - surface.roughness_factor));
        
        overall_colour += colour;
    }
    
    // apply emission component
    overall_colour += surface.emission_strength * surface_colour;
    
    colour = float4(overall_colour, 1);
    normal = surface_normal;
    depth = 1.0f - (1.0f / (varyings.position.w + 1.0f));
}

#endif