#pragma once

inline constexpr char jacobi_diffusion_frag[] = R"glsl(
#version 430 core
#line 5

uniform float dx2_nudt;

layout(binding = 0) uniform sampler2D new_velocity;
layout(binding = 1) uniform sampler2D old_velocity;

out vec4 color;

ivec2 px = ivec2(gl_FragCoord.xy);

#define v(X, Y) texelFetch(new_velocity, px + ivec2(X, Y), 0)

void main()
{
    vec4 v0 = texelFetch(old_velocity, px, 0);
    color = (v(-1, 0) + v(0, 1) + v(1, 0) + v(0, -1) + dx2_nudt * v0) / (4.0 + dx2_nudt);
})glsl";