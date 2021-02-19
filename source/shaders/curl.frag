#pragma once

inline constexpr char curl_frag[] = R"glsl(
#version 430 core
#line 5

uniform float half_inv_dx;

layout(binding = 0) uniform sampler2D velocity;

out vec4 curl;

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
    curl.x = half_inv_dx * (v(TX_R).y - v(TX_L).y - v(TX_T).x + v(TX_B).x);
})glsl";