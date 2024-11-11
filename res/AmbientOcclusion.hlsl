#define NUM_SAMPLES 64

#include "Dither.hlsl"

cbuffer AmbientOcclusionConstants : register(b0)
{
    float4x4 projection_matrix;
    float4x4 view_matrix;
    float4x4 view_matrix_inv;
    float4x4 projection_matrix_inv;
    float2 screen_size;
    float radius;
    float _;
    float4 samples[NUM_SAMPLES];
};

Texture2D normal : register(t0);
Texture2D depth  : register(t1);
SamplerState bilinear_sampler : register(s0);

struct Varyings
{
    float4 position  : SV_POSITION;
    float2 uv        : TEXCOORD0;
};

Varyings VS_main(float3 position : POSITION, float4 colour : COLOR, float3 normal : NORMAL, float2 uv : TEXCOORD0, float3 tangent : TANGENT0)
{
    Varyings output = (Varyings)0;
    output.position = float4(position.xy, 0, 1);
    output.uv = uv;
    
    return output;
}

float4 PS_main(Varyings input) : SV_TARGET
{
    float2 screen_uv = (input.uv * float2(0.5f, -0.5f)) + 0.5f;
    
    float3 world_normal = normal.Sample(bilinear_sampler, screen_uv).xyz;
    float3 view_normal = normalize(mul(float4(world_normal, 0.0f), view_matrix).xyz);
    
    float z = depth.Sample(bilinear_sampler, screen_uv).x;
    float4 clip_position = float4(input.uv, z, 1.0f);
    float4 view_position = mul(clip_position, projection_matrix_inv);
    view_position /= view_position.w;
    float linear_depth = -view_position.z;
    
    float2 pixel_uv = floor(screen_uv * screen_size);
    float dither = (dither_map_4[(pixel_uv.x % 4) + ((pixel_uv.y % 4) * 4)] / 16) * 2 - 1;
    
    float3 view_normal_perp_1 = view_normal.zxy;
    float3 view_normal_perp_2 = view_normal.yzx;
    float anti_dither = sqrt(1 - (dither * dither));
    
    float3 view_tangent = (dither * view_normal_perp_1) + (anti_dither * view_normal_perp_2);
    float3 view_bitangent = normalize(cross(view_tangent, view_normal));
    view_tangent = normalize(cross(view_bitangent, view_normal));
    float3x3 tbn = float3x3(view_tangent, view_bitangent, view_normal);
    
    float occlusion = 0.0f;
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        float3 view_sample = (mul(samples[i].xyz, tbn) * radius) + view_position.xyz;
        float4 sample_ndc = mul(float4(view_sample, 1.0f), projection_matrix);
        sample_ndc /= sample_ndc.w;
        float linear_sample_depth = -view_sample.z;
        
        if (abs(sample_ndc.x) > 1 || abs(sample_ndc.y) > 1)
            continue;
        
        float sample_z = depth.Sample(bilinear_sampler, (sample_ndc.xy * float2(0.5f, -0.5f)) + 0.5f).x;
        float4 resample_ndc = float4(view_sample.xy, sample_z, 1.0f);
        float4 view_resample = mul(resample_ndc, projection_matrix_inv);
        view_resample /= view_resample.w;
        float linear_resample_depth = -view_resample.z;
        
        float r = smoothstep(0.0f, 1.0f, radius / abs(linear_depth - linear_resample_depth));
        occlusion += ((linear_sample_depth > linear_resample_depth + 0.025f) ? 1.0f : 0.0f) * r;
    }
    
    occlusion = pow(smoothstep(0, 1, 1 - (occlusion / NUM_SAMPLES)), 3);
    
    return float4(occlusion, 0, 0, 1);
}