#pragma once

inline constexpr char vorticity_frag[] = R"glsl(
#version 430 core
#line 5

uniform float half_inv_dx;
uniform float dt;
uniform float vorticity_scale;

layout(binding = 0) uniform sampler2D velocity;
layout(binding = 1) uniform sampler2D curl;

out vec4 new_velocity;

in vec2 TX_C;
in vec2 TX_L;
in vec2 TX_R;
in vec2 TX_T;
in vec2 TX_B;

#define c(TX) texture(curl, TX).x

void main()
{
    vec2 force = half_inv_dx * vec2(abs(c(TX_T)) - abs(c(TX_B)), abs(c(TX_R)) - abs(c(TX_L)));
    float length2 = max(1e-4, dot(force, force));
    force *= inversesqrt(length2) * vorticity_scale * c(TX_C) * vec2(1, -1);
    new_velocity.xy = texture(velocity, TX_C).xy + force * dt;
})glsl";