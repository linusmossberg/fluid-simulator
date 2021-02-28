#pragma once

inline constexpr char gradient_subtract_frag[] = R"glsl(
#version 430 core
#line 5

uniform float half_inv_dx;

layout(binding = 0) uniform sampler2D pressure;
layout(binding = 1) uniform sampler2D velocity;

out vec4 new_velocity;

in vec2 C, L, R, T, B;

#define p(TX) texture(pressure, TX).x

void main()
{
    vec2 v = texture(velocity, C).xy;
    vec2 pressure_gradient = half_inv_dx * vec2(p(R) - p(L), p(T) - p(B));

    new_velocity.xy = v - pressure_gradient;
})glsl";