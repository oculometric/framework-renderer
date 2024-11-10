cbuffer PostProcessConstants : register(b0)
{
    float4x4 projection_matrix;
    float4x4 view_matrix;
    float4x4 view_matrix_inv;
    float4x4 projection_matrix_inv;
    float2 screen_size;
    float2 clipping_distances;
    float2 fog_start_end;
    float2 _;
    float fog_strength;
    float3 fog_colour;
    int output_mode;
};

Texture2D screen : register(t0);
Texture2D normal : register(t1);
Texture2D depth  : register(t2);
Texture2D ambient_occlusion  : register(t3);
TextureCube skybox : register(t4);
SamplerState bilinear_sampler : register(s0);
SamplerState nearest_sampler : register(s1);
Texture2D text_masks : register(t5);

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

float3 mix_screen(float3 a, float3 b, float3 t)
{
    return lerp(a, 1 - ((1 - a) * (1 - b)), t);
}

float3 mix_soft_light(float3 a, float3 b, float t)
{
    return lerp(a, ((1 - (2 * b)) * a * a) + (2 * b * a), t);
}

float3 mix_custom(float3 a, float3 b, float t)
{
    return lerp(a, ((a * b) + b) / 2, t);
}

float3 fog(float2 screen_uv)
{
    float2 uv = (screen_uv - 0.5f) * float2(2.0f, -2.0f);
    float4 clip_dir = normalize(float4(uv, 1, 1));
    float3 direction = mul(mul(clip_dir, projection_matrix_inv) * float4(1, 1, 1, 0), view_matrix_inv).xyz * float3(-1, 1, 1);
    
    float3 skybox_sample = skybox.Sample(bilinear_sampler, direction.xzy).rgb * 0.8f;
    float f = depth.Sample(bilinear_sampler, screen_uv).r;
    float4 clip_pos = float4(uv, f, 1);
    float4 view_pos = mul(clip_pos, projection_matrix_inv);
    view_pos /= view_pos.w;
    f = length(view_pos.xyz);
    float3 scene_sample = screen.Sample(bilinear_sampler, screen_uv);
    float fog_mix = clamp(pow(clamp((f - fog_start_end.r) / (fog_start_end.g - fog_start_end.r), 0, 1), 0.8), 0, 1);
    float3 fogged = mix_custom(scene_sample, fog_colour, fog_mix * fog_strength);
    
    return (clipping_distances.g - f) <= 10.0f ? skybox_sample : fogged;
}

float3 sharpen(float2 uv, float2 pixels_in_image)
{
    float2 step = 1.0f / pixels_in_image;
    
    const float kernel[] =
    {
        -0.5f, -1.0f, -0.5f,
        -1.0f, 7.0f, -1.0f,
        -0.5f, -1.0f, -0.5f
    };
    
    return (fog(uv + (step * float2(-1, -1))).rgb * kernel[0])
         + (fog(uv + (step * float2(0, -1))).rgb * kernel[1])
         + (fog(uv + (step * float2(1, -1))).rgb * kernel[2])
         + (fog(uv + (step * float2(-1, 0))).rgb * kernel[3])
         + (fog(uv + (step * float2(0, 0))).rgb * kernel[4])
         + (fog(uv + (step * float2(1, 0))).rgb * kernel[5])
         + (fog(uv + (step * float2(-1, 1))).rgb * kernel[6])
         + (fog(uv + (step * float2(0, 1))).rgb * kernel[7])
         + (fog(uv + (step * float2(1, 1))).rgb * kernel[8]);
}

float4 PS_main(Varyings input) : SV_TARGET
{
    float2 screen_uv = (input.uv / float2(2.0f, -2.0f)) + 0.5f;
    
    float ao = ambient_occlusion.Sample(bilinear_sampler, screen_uv).r;
    return float4(screen_uv, ao, 1);
    
    // text shader
    float2 text_resolution = screen_size / 8.0f;
    float2 text_uv = screen_uv * text_resolution;
    // sample the pixel (floored for pixelation) and fog it
    float3 colour = fog((floor(text_uv) + 0.5f) / text_resolution).rgb;
    
    // number of colour luminosity divisions
    const float divs = 4.0f;
    // fraction between the lower colour bound and the upper colour bound, in each channel
    float3 frc = frac(colour * divs);
    
    const float3 luma_vector = float3(0.2126, 0.7152, 0.0722);
    
    // float which we use to select the character to use. as the luminosity of the fraction increases, we use a higher-fill-percentage character for dithering
    float character_selector = dot(frc, luma_vector) * 16.0f;
    // offset into the text atlas based on the character selector
    float2 text_offset = float2(floor(character_selector) % 4.0f, floor(character_selector / 4.0f));
    // sample the text atlas and use it as a mask to blend between the rounded-down colour and the rounded-up colour
    float mask = text_masks.Sample(nearest_sampler, (frac(text_uv) + text_offset) / 4.0f).r;
    float3 text_blended = exp(mask < 0.5f ? floor(log(colour) * divs) / divs : ceil(log(colour) * divs) / divs);
    
    // output depending on mode
    switch (output_mode)
    {
        case 0:
            return float4(text_blended, 1);
        case 1:
            return float4(screen.Sample(bilinear_sampler, screen_uv).rgb, 1);
        case 2:
            return float4(normal.Sample(bilinear_sampler, screen_uv).rgb, 1);
        case 3:
            float f = depth.Sample(bilinear_sampler, screen_uv).r;
            float4 clip_pos = float4(input.uv, f, 1);
            float4 view_pos = mul(clip_pos, projection_matrix_inv);
            view_pos /= view_pos.w;
            f = length(view_pos.xyz);
            return float4((f.rrr - clipping_distances.r) / (clipping_distances.g - clipping_distances.r), 1);
        case 4:
            return float4(sharpen(screen_uv, screen_size), 1);
    }
    
    return float4(0, 0, 0, 1);
}