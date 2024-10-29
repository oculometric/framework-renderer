cbuffer ConstantBuffer : register(b0)
{
    float4x4 projection_matrix;
    float4x4 view_matrix;
    float4x4 view_matrix_inv;
    float4x4 projection_matrix_inv;
}

Texture2D screen : register(t0);
Texture2D normal : register(t1);
Texture2D depth  : register(t2);
TextureCube skybox : register(t3);
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

float4 sharpen(Texture2D tex, float2 uv, float2 pixels_in_image)
{
    float2 step = 1.0f / pixels_in_image;
    
    const float kernel[] =
    {
        -0.5f, -1.0f, -0.5f,
        -1.0f,  7.0f, -1.0f,
        -0.5f, -1.0f, -0.5f
    };
    
    return (tex.Sample(bilinear_sampler, uv + (step * float2(-1, -1))) * kernel[0])
         + (tex.Sample(bilinear_sampler, uv + (step * float2( 0, -1))) * kernel[1])
         + (tex.Sample(bilinear_sampler, uv + (step * float2( 1, -1))) * kernel[2])
         + (tex.Sample(bilinear_sampler, uv + (step * float2(-1,  0))) * kernel[3])
         + (tex.Sample(bilinear_sampler, uv + (step * float2( 0,  0))) * kernel[4])
         + (tex.Sample(bilinear_sampler, uv + (step * float2( 1,  0))) * kernel[5])
         + (tex.Sample(bilinear_sampler, uv + (step * float2(-1,  1))) * kernel[6])
         + (tex.Sample(bilinear_sampler, uv + (step * float2( 0,  1))) * kernel[7])
         + (tex.Sample(bilinear_sampler, uv + (step * float2( 1,  1))) * kernel[8]);
}

float4 PS_main(Varyings input) : SV_TARGET
{
    float2 screen_uv = (input.uv / float2(2.0f, -2.0f)) + 0.5f;
    
    float2 pixels = float2(1280, 960) / 3.0f;
    
    float2 pixelated_uv = ceil(screen_uv * pixels) / pixels;
    
    float4 clip_dir = normalize(float4(input.uv, 1, 1));
    float3 direction = mul(mul(clip_dir, projection_matrix_inv) * float4(1, 1, 1, 0), view_matrix_inv).xyz * float3(-1, 1, 1);
    
    float4 colour_sample = screen.Sample(bilinear_sampler, pixelated_uv);
    float4 skybox_sample = skybox.Sample(bilinear_sampler, direction.xzy);
    float f = depth.Sample(bilinear_sampler, pixelated_uv).r;
    float4 depth_sample = float4(f,f,f,1.0f);
    float4 normal_sample = normal.Sample(bilinear_sampler, pixelated_uv);
    float4 sharpened_sample = sharpen(screen, pixelated_uv, pixels);
    colour_sample = f > 0.99f ? skybox_sample : colour_sample;
    sharpened_sample = f > 0.99f ? skybox_sample : sharpened_sample;
    
    float mixer = ((screen_uv.x + screen_uv.y * -0.2f) + 1.0f) % 1.0f;
 
    if (mixer < 0.25f)
        return sharpened_sample;
    else if (mixer < 0.5f)
        return colour_sample;
    else if (mixer < 0.75f)
        return depth_sample;
    else
        return normal_sample;
}