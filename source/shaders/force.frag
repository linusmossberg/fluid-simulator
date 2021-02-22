#pragma once

inline constexpr char force_frag[] = R"glsl(
#version 430 core
#line 5

uniform vec2 pos;
layout(binding = 0) uniform sampler2D velocity;

uniform vec2 force;
uniform float dt;
uniform vec2 tx_size;

in vec2 TX_C;

out vec4 new_velocity;

#define PI 3.1415926535897932384626433832795

void main()
{
    new_velocity.xy = texture(velocity, TX_C).xy;

    vec2 d = (TX_C - pos) / tx_size;

    const float radius = 10.0;
    const float falloff_integral = PI / radius;
    
    float falloff = exp(-dot(d, d) / radius);

    new_velocity.xy += dt * (falloff / falloff_integral) * force;
})glsl";