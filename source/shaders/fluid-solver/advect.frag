#pragma once

inline constexpr char advect_frag[] = R"glsl(
#version 430 core
#line 5

uniform float dt;
uniform vec2 world2tx; // World space to texture space

layout(binding = 0) uniform sampler2D velocity;
layout(binding = 1) uniform sampler2D quantity;

in vec2 TX_C;

out vec4 color;

vec2 v(vec2 tx)
{
    vec2 vel = texture(velocity, tx).xy;

    if(tx.x < 0.0 || tx.x > 1.0) vel.x = -vel.x;
    if(tx.y < 0.0 || tx.y > 1.0) vel.y = -vel.y;

    return vel;
}

vec2 RK4()
{
    vec2 k1 = v(TX_C);
    vec2 k2 = v(TX_C - world2tx * 0.5 * k1 * dt);
    vec2 k3 = v(TX_C - world2tx * 0.5 * k2 * dt);
    vec2 k4 = v(TX_C - world2tx * k3 * dt);
    return TX_C - world2tx * dt * (k1 + 2.0 * (k2 + k3) + k4) / 6.0;
}

void main()
{
    // Backward integrate to previous position using Runge-Kutta 4
    color = texture(quantity, RK4());
})glsl";