#include "Common.hlsl"

cbuffer ConstantBuffer : register(b0)
{
    float cell_size;
    float3 sky_colour;
    float individual_size;
    float shape;
    int iterations;
}

COMMON_CONSTANT_BUFFER;         // defines a second constant buffer in register b1 with the variable 'common'

struct Varyings
{
    float4 position         : SV_POSITION;  // clip space position
    float3 world_position   : COLOUR1;
    float3 world_normal     : NORMAL;
};

Texture2D cloud_atlas : register(t0);
SamplerState bilinear_sampler : register(s0);

Varyings VS_main(float3 position : POSITION, float4 colour : COLOR, float3 normal : NORMAL, float2 uv : TEXCOORD0, float3 tangent : TANGENT0)
{
    Varyings output = (Varyings)0;
    
    float3 wp;
    float3 vp;
    
    output.position = modelProjectionTransformation(position, common.world_matrix, output.world_position, common.view_matrix, vp, common.projection_matrix);
    output.world_normal = mul(float4(normal, 0), common.world_matrix).xyz;
    
    return output;
}

float hash(float3 v)
{
    return frac(sin(dot(v, float3(201.0f, 123.0f, 304.2f))) * 190493.02095f) * 2.0f - 1.0f;
}

float3 screen(float3 v_a, float3 v_b, float m)
{
    return lerp(v_a, 1 - ((1 - v_a) * (1 - v_b)), m);
}

Fragment PS_main(Varyings input)
{
    Fragment frag = (Fragment)0;
    
    frag.normal = float4(normalize(input.world_normal), 1);
    float2 triplanar = (abs(frag.normal.x) > 0.707 ? input.world_position.yz : (abs(frag.normal.z) > 0.707 ? input.world_position.xy : input.world_position.xz)) * -1;
    float2 uv = ((triplanar / cell_size) + 0.5f);
    uv.y += (floor(uv.x) % 2.0f) * 0.5f;

    float3 working_colour = sky_colour;

	// clouds are generated in square cells, with a world size of cell_size
    float2 tile = floor(uv);
    float2 frc = frac(uv);

    for (float i = 0.0f; i < float(iterations); i += 1.0f)
    {
        float2 voronoi_offset = float2(hash(float3(i, tile.x, tile.y)), hash(float3(tile.y, i, tile.x)));
        voronoi_offset.y = (shape / (sqrt(shape + 1.0f) - voronoi_offset.y)) - sqrt(shape + 1.0f);
        voronoi_offset.x /= clamp(voronoi_offset.y, 0.0f, 1.0f) + 1.0f;
        voronoi_offset.y += shape / (shape + 1.0f);
        voronoi_offset.y *= -1.0f;
        float2 center = (voronoi_offset + 1.0f) * 0.25f;
        float layer = ((hash(float3(i * 55.245f, tile.x * 12.463f, tile.y * 245.535f)) * 0.5f) + 0.5f) * 36;
        float2 sample_position = saturate(((((frc + 0.25f - center) * 2.0f - 1.0f) / individual_size) + 1.0f) / 2.0f);
        float2 texture_uv = (sample_position / 6) + (float2(floor(layer) % 6, floor(layer / 6) % 6) / 6);
        float4 tex = cloud_atlas.Sample(bilinear_sampler, texture_uv);
        working_colour = screen(working_colour, tex.rgb, tex.w * length(tex.rgb));
    }

    frag.colour = float4(working_colour * 0.8f, 1);
    
    return frag;
}
