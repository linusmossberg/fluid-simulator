#pragma once

inline constexpr char gradient_subtract_frag[] = R"glsl(
#version 430 core
#line 5

uniform float half_inv_dx;

layout(binding = 0) uniform sampler2D pressure;
layout(binding = 1) uniform sampler2D velocity;

out vec4 new_velocity;

ivec2 px = ivec2(gl_FragCoord.xy);

#define p(X, Y) texelFetch(pressure, px + ivec2(X, Y), 0).x

void main()
{
    vec2 v = texelFetch(velocity, px, 0).xy;

    vec2 pressure_gradient = half_inv_dx * vec2(p(1, 0) - p(-1, 0), p(0, 1) - p(0, -1));
    new_velocity.xy = v - pressure_gradient;
})glsl";