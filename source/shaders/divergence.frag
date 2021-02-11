#pragma once

inline constexpr char divergence_frag[] = R"glsl(
#version 430 core
#line 5

uniform float half_inv_dx;

layout(binding = 0) uniform sampler2D velocity;

out vec4 divergence;

ivec2 px = ivec2(gl_FragCoord.xy);

#define v(X, Y) texelFetch(velocity, px + ivec2(X, Y), 0)

void main()
{
    float L = v(-1, 0).x;
    float R = v(1, 0).x;
    float B = v(0, -1).y;
    float T = v(0, 1).y;

    vec2 C = v(0, 0).xy;

    ivec2 mx = textureSize(velocity, 0) - 1;

    if(px.x <= 0) { L = -C.x; }
    if(px.x >= mx.x) { R = -C.x; }
    if(px.y <= 0) { B = -C.y; }
    if(px.y >= mx.y) { T = -C.y; }

    divergence.x = half_inv_dx * (R - L + T - B);
})glsl";