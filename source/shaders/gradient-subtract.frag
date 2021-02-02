#pragma once

inline constexpr char gradient_subtract_frag[] = R"glsl(
#version 430 core
#line 5

uniform vec2 texel_size;
uniform float dx;

layout(binding = 0) uniform sampler2D pressure;
layout(binding = 1) uniform sampler2D velocity;

in vec2 interpolated_texcoord;

out vec4 new_velocity;

float p(float x_offset, float y_offset)
{
    return texture(pressure, (interpolated_texcoord + vec2(x_offset, y_offset) * texel_size)).x;
}

vec2 pressureGradient()
{
    return (0.5 / dx) * vec2(p(1, 0) - p(-1, 0), p(0, 1) - p(0, -1));
}

void main()
{
    vec2 v = texture(velocity, interpolated_texcoord).xy;
    new_velocity.xy = v - pressureGradient();
    new_velocity.zw = vec2(0);
})glsl";