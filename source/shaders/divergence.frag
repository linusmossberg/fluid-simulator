#pragma once

inline constexpr char divergence_frag[] = R"glsl(
#version 430 core
#line 5

uniform float half_inv_dx;

layout(binding = 0) uniform sampler2D velocity;

out vec4 divergence;

ivec2 px = ivec2(gl_FragCoord.xy);

#define v(X, Y) texelFetch(velocity, px + ivec2(X, Y), 0).xy

void main()
{
    divergence.x = half_inv_dx * (v(1, 0).x - v(-1, 0).x + v(0, 1).y - v(0, -1).y);
})glsl";