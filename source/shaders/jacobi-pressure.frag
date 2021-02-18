#pragma once

inline constexpr char jacobi_pressure_frag[] = R"glsl(
#version 430 core
#line 5

uniform float dx2;

// laplace(pressure) = divergence
layout(binding = 0) uniform sampler2D pressure;
layout(binding = 1) uniform sampler2D divergence;

out vec4 color;

in vec2 TX_C;
in vec2 TX_L;
in vec2 TX_R;
in vec2 TX_T;
in vec2 TX_B;

#define p(TX) texture(pressure, TX).x

void main()
{
    vec4 div = texture(divergence, TX_C);
    color = (p(TX_L) + p(TX_R) + p(TX_B) + p(TX_T) - dx2 * div) / 4.0;
})glsl";