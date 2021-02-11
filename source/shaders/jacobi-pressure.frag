#pragma once

inline constexpr char jacobi_pressure_frag[] = R"glsl(
#version 430 core
#line 5

uniform float dx2;

// laplace(pressure) = divergence
layout(binding = 0) uniform sampler2D pressure;
layout(binding = 1) uniform sampler2D divergence;

out vec4 color;

ivec2 px = ivec2(gl_FragCoord.xy);

#define p(X, Y) texelFetch(pressure, px + ivec2(X, Y), 0)

void main()
{
    float L = p(-1, 0).x;
    float R = p(1, 0).x;
    float B = p(0, -1).x;
    float T = p(0, 1).x;

    float C = p(0, 0).x;

    ivec2 mx = textureSize(pressure, 0) - 1;

    if(px.x <= 0) { L = C; }
    if(px.x >= mx.x) { R = C; }
    if(px.y <= 0) { B = C; }
    if(px.y >= mx.y) { T = C; }

    vec4 div = texelFetch(divergence, px, 0);
    color = (L + R + B + T - dx2 * div) / 4.0;
})glsl";