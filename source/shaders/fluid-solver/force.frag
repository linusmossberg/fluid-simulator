#pragma once

inline constexpr char force_frag[] = R"glsl(
#version 430 core
#line 5

uniform vec2 pos;
layout(binding = 0) uniform sampler2D velocity;

uniform vec2 force;
uniform float dt;
uniform vec2 tx_size;
uniform vec2 world2tx;

in vec2 C;

out vec4 new_velocity;

#define PI 3.1415926535897932384626433832795

vec2 F(vec2 x)
{
    vec2 aspect = vec2(1.0, tx_size.x / tx_size.y) * 256.0;
    vec2 d = (x - pos) * aspect;
    const float radius = 10.0;
    const float falloff_integral = PI / radius;
    float falloff = exp(-dot(d, d) / radius);
    return (falloff / falloff_integral) * force;
}

void main()
{
    new_velocity.xy = texture(velocity, C).xy + dt * F(C);
})glsl";