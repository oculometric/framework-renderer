#if !defined(PBR_H)
#define PBR_H

#include "Light.hlsl"
#include "Common.hlsl"
#include "Dither.hlsl"

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
    bool using_triplanar;
    float triplanar_scale;
};

struct PBRVaryings
{
    float4 position;            // clip space position of the fragment
    float3 view_position;       // view space position of the fragment
    float3 world_position;      // world space position of the fragment
    float3 normal;              // world space normal of the fragment
    float2 uv;                  // uv coordinate of the fragment
    float3x3 tbn;               // tangent-bitangent-normal matrix
};

struct PBRTextures
{
    Texture2D albedo;           // texture to be sampled for colour data
    Texture2D normal;           // texture to be sampled for normal data
    Texture2DArray shadow_map;  // stores shadows for the lights
    SamplerState texture_sampler;
};

void evaluateSurface(PBRSurface surface, PBRTextures textures, PBRConstants constants, PBRVaryings varyings, out float4 colour, out float3 normal)
{
    // calculate direciton from the camera to the target fragment
    float3 view_dir = normalize(mul(float4(normalize(varyings.view_position), 0), constants.view_matrix_inv).xyz);
    
    float3 surface_normal = normalize(varyings.normal);
    
    // triplanar mapping, if the material enables it
    float2 uv = varyings.uv;
    if (constants.using_triplanar)
        uv = (abs(surface_normal.x) > 0.707 ? varyings.world_position.yz : (abs(surface_normal.z) > 0.707 ? varyings.world_position.xy : varyings.world_position.xz)) * constants.triplanar_scale * -1.0f;
        
    // colour from the texture
    float4 texture_colour = textures.albedo.Sample(textures.texture_sampler, uv);
    // surface colour is the product of albedo (!!!) texture colour and base colour
    float3 surface_colour = texture_colour.rgb * surface.base_colour.rgb;
    // test alpha
    int2 coord = int2(varyings.position.xy);
    if (!dither_4x4(texture_colour.a, coord))
        discard;
    
    // normal from the texture. possibly this is just blank
    float3 texture_normal = (textures.normal.Sample(textures.texture_sampler, uv).xyz * 2.0f) - 1.0f;
    // actual normal we're going to use for lighting
    if (length(texture_normal) <= 1.5f)
        surface_normal = normalize(lerp(surface_normal, mul(texture_normal, varyings.tbn), surface.normal_strength)); // always make sure you have your multiplications the right way round, bucko!
    normal = surface_normal;
    
    float3 overall_colour = lerp(float3(0,0,0), constants.light_ambient.rgb * surface_colour, surface.roughness_factor);
    for (uint i = 0; i < NUM_LIGHTS; i++)
    {
        Light light = constants.lights[i];
        
        // if light has zero strength, skip it, it's probably disabled
        if (light.strength == 0) continue;
        
        // sample shadow map
        float4 reprojected_point = mul(float4(varyings.world_position, 1), light.light_matrix);
        reprojected_point /= reprojected_point.w;
        float point_light_depth = reprojected_point.z;
        
        float2 reprojected_uv = (reprojected_point.xy * float2(0.5f, -0.5f)) + 0.5f;
        if (reprojected_uv.x >= 0 && reprojected_uv.x <= 1 && reprojected_uv.y >= 0 && reprojected_uv.y <= 1)
        {
            float shadow_depth = textures.shadow_map.Sample(textures.texture_sampler, float3(reprojected_uv, i)).r;
            if ((point_light_depth - shadow_depth) > 0.001f)
                continue;
        }
        
        // direction of the light in world space
        float3 light_dir;
        float light_strength = light.strength;
        if (light.light_direction.w > 0)
        {
            float3 light_vec = varyings.world_position - light.light_position;
            light_dir = normalize(light_vec);
            light_strength /= (length(light_vec) * length(light_vec)) + 1;
            
            if (light.angle < 180)
                light_strength *= pow(saturate((light.angle - degrees(acos(dot(light_dir, light.light_direction.xyz)))) / light.angle), 0.5);
        }
        else
        {
            light_dir = normalize(light.light_direction.xyz);
        }
        // light colour with strength applied
        float3 light_col = light.colour.rgb * light_strength;
        
        // light direction
        float3 w_i = -light_dir;
        // view direction
        float3 w_o = -view_dir;
        // surface normal
        float3 n = surface_normal;
        // roughness factor
        float a = surface.roughness_factor;
        
        // dot product between the light direction and the surface normal
        float wi_dot_n = dot(w_i, n);
        // dot product between the view direction and the surface normal
        float wo_dot_n = dot(w_o, n);
        // vector which is halfway between the light vector and surface normal
        float3 h = normalize(w_i + n);
        
        // Trowbridge-Reitz GGX normal distribution function, ref https://learnopengl.com/PBR/Theory
        float n_dot_h = dot(n, h);
        float a2 = a * a;
        float d = (max(n_dot_h, 0.0f) * max(n_dot_h, 0.0f) * (a2 - 1.0f)) + 1.0f;
        float ndf_trggx = a2 / (3.14159f * d * d);
        
        // Schlick GGX geometry function, ref as above
        float k = ((a + 1) * (a + 1)) / 8;
        float gf_sggx = (saturate(wo_dot_n) / ((saturate(wo_dot_n) * (1.0f - k)) + k))
                      * (saturate(wi_dot_n) / ((saturate(wi_dot_n) * (1.0f - k)) + k)); // modified to add saturate functions
        
        // Fresnel-Schlick fresnel approximation, ref as above
        float3 f0 = lerp(float3(0.04f, 0.04f, 0.04f), surface_colour, surface.metallic_factor);
        float3 ff_fs = f0 + ((1.0f - f0) * pow(saturate(1.0f - max(n_dot_h, 0.0f)), 5.0f));
        
        // Cook-Torrance specular BRDF, ref as above
        float3 f_ct = (ndf_trggx * gf_sggx * ff_fs) / ((4.0f * max(wi_dot_n, 0.0f) * max(wo_dot_n, 0.0f)) + 0.0001f);
        
        // Lambertian diffuse BRDF, ref as above
        float3 f_lambert = surface_colour / 3.14159f;
        
        float3 k_s = ff_fs;
        float3 k_d = float3(1.0f, 1.0f, 1.0f) - k_s;
        k_d *= 1.0f - surface.metallic_factor;
        
        // Cook-Torrance BRDF (combined), ref as above
        float3 f_r = ((k_d * f_lambert) + (f_ct));
        
        float3 l = f_r * saturate(wi_dot_n) * light_col;
        overall_colour += l;
    }
    
    // apply emission component
    overall_colour += surface.emission_strength * texture_colour;
    
    colour = float4(overall_colour, 1);
}

#endif