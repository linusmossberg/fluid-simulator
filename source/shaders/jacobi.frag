#pragma once

inline constexpr char jacobi_frag[] = R"glsl(
#version 430 core
#line 5

uniform float alpha;
uniform float inv_beta;

// Ax = b
layout(binding = 0) uniform sampler2D x_tex;
layout(binding = 1) uniform sampler2D b_tex;

out vec4 color;

ivec2 px = ivec2(gl_FragCoord.xy);

#define x(X, Y) texelFetch(x_tex, px + ivec2(X, Y), 0)

void main()
{
    vec4 b = texelFetch(b_tex, px, 0);
    color = (x(-1, 0) + x(0, 1) + x(1, 0) + x(0, -1) + alpha * b) * inv_beta;
})glsl";