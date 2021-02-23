#pragma once

inline constexpr char divergence_frag[] = R"glsl(
#version 430 core
#line 5

uniform float half_inv_dx;

layout(binding = 0) uniform sampler2D velocity;

out vec4 divergence;

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
    divergence.x = half_inv_dx * (v(TX_R).x - v(TX_L).x + v(TX_T).y - v(TX_B).y);
})glsl";