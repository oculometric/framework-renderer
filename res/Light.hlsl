#if !defined(LIGHT_H)
#define LIGHT_H

struct Light
{
    float3 colour;              // colour of the light
    float strength;             // brightness multiplier
    
    float4 light_direction;     // direction the light is pointing in. W component being 1 indiciates that the light is positional, 0 indicates directional
    
    float3 light_position;      // position of the light, only relevant for positional lights
    float angle;                // angle of the light in sin(degrees). angle = 0 will disable the light, angle = 1 will illuminate in a hemisphere, angle = -1 makes it a point light (as opposed to a spot light)
    
    float4x4 light_matrix;      // world-to-clip matrix for the light
    // TODO: maybe implement falloff (distance and angle) control for this?
};

#endif