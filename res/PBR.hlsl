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
    float3 view_dir = normalize(mul(float4(normalize(varyings.view_position), 1), constants.view_matrix_inv).xyz);
    
    // colour from the texture
    float4 texture_colour = textures.albedo.Sample(textures.texture_sampler, varyings.uv);
    // surface colour is the product of albedo (!!!) texture colour and base colour
    float3 surface_colour = texture_colour.rgb * surface.base_colour.rgb;
    // test alpha
    int2 coord = int2(varyings.position.xy);
    if (!dither_4x4(texture_colour.a, coord))
        discard;
    
    // normal from the texture. possibly this is just blank
    float3 texture_normal = (textures.normal.Sample(textures.texture_sampler, varyings.uv).xyz * 2.0f) - 1.0f;
    // actual normal we're going to use for lighting
    float3 surface_normal = normalize(varyings.normal);
    if (length(texture_normal) <= 1.5f)
        surface_normal = lerp(surface_normal, mul(texture_normal, varyings.tbn), surface.normal_strength); // always make sure you have your multiplications the right way round, bucko!
    normal = surface_normal;
    
    float3 overall_colour = lerp(float3(0,0,0), constants.light_ambient.rgb * surface_colour, surface.roughness_factor);
    for (uint i = 0; i < NUM_LIGHTS; i++)
    {
        Light light = constants.lights[i];
        
        // sample shadow map
        float4 reprojected_point = mul(float4(varyings.world_position, 1), light.light_matrix);
        reprojected_point /= reprojected_point.w;
        float point_light_depth = reprojected_point.z;
        
        float2 reprojected_uv = (reprojected_point.xy * float2(0.5f, -0.5f)) + 0.5f;
        if (reprojected_uv.x < 0 || reprojected_uv.x > 1)
            continue;
        if (reprojected_uv.y < 0 || reprojected_uv.y > 1)
            continue;
        
        float shadow_depth = textures.shadow_map.Sample(textures.texture_sampler, float3(reprojected_uv, i)).r;
        //colour = float4(reprojected_uv, 0.0f, 1.0);
        //colour = (reprojected_point * 0.5f) + 0.5f;
        //colour = float4(point_light_depth, point_light_depth, point_light_depth, 1);
        //colour = float4(shadow_depth, shadow_depth, shadow_depth, 1);
        //colour = textures.shadow_map.Sample(textures.texture_sampler, float3(varyings.uv - 0.25f, i));
        //return;
        if ((point_light_depth - shadow_depth) > 0.001f)
            continue;
        
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
        // dot product between the light direction and the surface normal
        float dir_dot_norm = dot(-light_dir, surface_normal);
        
        //float3 halfway_vec = normalize(-light_dir + -view_dir);
        
        //// Trowbridge-Reitz GGX normal distribution function, ref https://learnopengl.com/PBR/Theory
        //float n_dot_h = saturate(dot(surface_normal, halfway_vec));
        //float d = (n_dot_h * n_dot_h * ((surface.roughness_factor * surface.roughness_factor) - 1)) + 1;
        //float trggx = (surface.roughness_factor * surface.roughness_factor) / (3.14159 * d * d);
        
        //// Schlick GGX geometry function, ref as above
        //float k = (surface.roughness_factor + 1);
        //k *= k;
        //float sggx = (dot(surface_normal, -view_dir) / ((dot(surface_normal, -view_dir) * (1 - k)) + k))
        //           * (dot(surface_normal, -light_dir) / ((dot(surface_normal, -light_dir) * (1 - k)) + k));
        
        //overall_colour += sggx;
        if (surface.roughness_factor > 0)
        {
            // diffuse component of the light's contribution
            float3 diffuse_light = saturate(dir_dot_norm) * light_col * surface_colour;
            overall_colour += diffuse_light * surface.roughness_factor;
        }
        
        if (surface.roughness_factor < 1)
        {
            // specular exponent. the higher the roughness, the more spread-out the specular highlight should be (and the less it should contribute)
            // TODO: make this physically accurate (energy-preserving)
            float specular_exp = 10.0f * (1.0f - surface.roughness_factor);
            // specular component of the light's contribution
            float specular_highlight = pow(saturate(dot(reflect(view_dir, surface_normal), light_dir)), specular_exp) * (dir_dot_norm > 0.0f);
            float3 specular_colour = lerp(specular_colour.rgb, surface_colour, surface.metallic_factor);
            float3 specular_light = specular_highlight * light_col * surface_colour;
            // TODO: as above, energy preservation
            overall_colour += specular_light * (1.0f - surface.roughness_factor);
        }
    }
    
    // apply emission component
    overall_colour += surface.emission_strength * surface_colour;
    
    colour = float4(overall_colour, 1);
}

#endif