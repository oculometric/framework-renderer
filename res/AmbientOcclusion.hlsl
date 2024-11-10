#define NUM_SAMPLES 64

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

float linearise(float d, float near, float far)
{
    return (2.0f * near) / ((far + near) - (d * (far - near)));
}

float4 PS_main(Varyings input) : SV_TARGET
{
    float2 screen_uv = (input.uv * float2(0.5f, -0.5f)) + 0.5f;
    
    // view space normal
    float3 n = normalize(mul(float4(normal.Sample(bilinear_sampler, screen_uv).xyz, 0.0f), view_matrix)).xyz;
    // depth from depth buffer
    float d = depth.Sample(bilinear_sampler, screen_uv).x;
    // linearised depth
    float l = linearise(d, 1.0f, 0.01f);
    // view space position
    float4 ndc = float4(input.position.xy, d, 1.0f);
    float4 p = mul(ndc, projection_matrix_inv);
    p /= p.w;
    // random vector
    float3 v = normalize((screen_uv.x * float3(345.1456, -1243.6403, -105.2631)) + (screen_uv.y * float3(102.4677, -2357.3530, 23.0455)));
    // random tangent
    float3 t = normalize(cross(n, n+v));
    // random bitangent
    float3 b = normalize(cross(t, n));
    // tangent bitangent normal matrix
    float3x3 tbn = float3x3(t, b, -n);
    
    // sample occlusion
    float occlusion = 0.0f;
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        // sample position
        float3 s = (mul(samples[i].xyz, tbn) * radius) + p.xyz;
        // sample position in clip space
        float4 ws = mul(float4(s, 1.0f), projection_matrix);
        ws /= ws.w;
        // sample depth
        float sd = depth.Sample(bilinear_sampler, (ws.xy * float2(0.5f, -0.5f)) + 0.5f).x;
        float sl = linearise(sd, 1.0f, 0.01f);
        
        // test occlusion
        float r = 1.0f; //float r = smoothstep(0.0f, 1.0f, radius / abs(l - sl));
        occlusion += ((sl >= l + 0.025f) ? 1.0f : 0.0f) * r;
    }
    
    occlusion = 1.0f - (occlusion / NUM_SAMPLES);
    
    return float4(occlusion, 0, 0, 1);
}