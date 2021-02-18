#pragma once

inline constexpr char gradient_subtract_frag[] = R"glsl(
#version 430 core
#line 5

uniform float half_inv_dx;

layout(binding = 0) uniform sampler2D pressure;
layout(binding = 1) uniform sampler2D velocity;

out vec4 new_velocity;

ivec2 px = ivec2(gl_FragCoord.xy);

in vec2 TX_C;
in vec2 TX_L;
in vec2 TX_R;
in vec2 TX_T;
in vec2 TX_B;

#define p(TX) texture(pressure, TX).x

void main()
{
    vec2 v = texture(velocity, TX_C).xy;
    vec2 pressure_gradient = half_inv_dx * vec2(p(TX_R) - p(TX_L), p(TX_T) - p(TX_B));

    new_velocity.xy = v - pressure_gradient;
})glsl";