#pragma once

inline constexpr char jacobi_diffusion_frag[] = R"glsl(
#version 430 core
#line 5

uniform float dx2_nudt;

layout(binding = 0) uniform sampler2D velocity;
layout(binding = 1) uniform sampler2D old_velocity;

out vec4 color;

in vec2 TX_C;
in vec2 TX_L;
in vec2 TX_R;
in vec2 TX_T;
in vec2 TX_B;

vec2 v(vec2 tx)
{
    vec2 vel = texture(velocity, tx).xy;

    if(tx.x < 0.0 || tx.x > 1.0) vel.x = -vel.x;
    if(tx.y < 0.0 || tx.y > 1.0) vel.y = -vel.y;

    return vel;
}

void main()
{
    vec2 v0 = texture(old_velocity, TX_C).xy;
    color.xy = (v(TX_L) + v(TX_T) + v(TX_R) + v(TX_B) + dx2_nudt * v0) / (4.0 + dx2_nudt);
})glsl";