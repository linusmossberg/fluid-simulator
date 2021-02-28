#pragma once

inline constexpr char jacobi_pressure_frag[] = R"glsl(
#version 430 core
#line 5

uniform float dx2;

// laplace(pressure) = divergence
layout(binding = 0) uniform sampler2D pressure;
layout(binding = 1) uniform sampler2D divergence;

out vec4 color;

in vec2 C, L, R, T, B;

#define p(TX) texture(pressure, TX).x

void main()
{
    vec4 div = texture(divergence, C);
    color = (p(L) + p(R) + p(B) + p(T) - dx2 * div) / 4.0;
})glsl";