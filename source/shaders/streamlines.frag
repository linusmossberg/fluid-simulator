#pragma once

inline constexpr char streamlines_frag[] = R"glsl(
#version 430 core
#line 5

uniform float inv_dx;

layout(binding = 0) uniform sampler2D velocity;
layout(binding = 1) uniform sampler2D noise;

in vec2 TX_C;

out vec4 color;

vec2 v(vec2 tx)
{
    tx += TX_C;
    
    vec2 vel = texture(velocity, tx).xy;

    if(tx.x < 0.0 || tx.x > 1.0) vel.x = -vel.x;
    if(tx.y < 0.0 || tx.y > 1.0) vel.y = -vel.y;

    return vel;
}

vec2 RK4(float dt, vec2 x0, float scale)
{
    vec2 k1 = inv_dx * texture(velocity, x0).xy;
    vec2 k2 = inv_dx * v(scale * 0.5 * k1 * dt);
    vec2 k3 = inv_dx * v(scale * 0.5 * k2 * dt);
    vec2 k4 = inv_dx * v(scale * k3 * dt);
    return x0 + scale * dt * (k1 + 2.0 * (k2 + k3) + k4) / 6.0;
}

void main()
{
    const int N = 25;
    const float trace_time = 0.25;
    const float dt = trace_time / N;

    vec2 x0 = TX_C;
    float avg_noise_back = 0.0;
    for(int i = 0; i < N; i++)
    {
        avg_noise_back += texture(noise, x0).x;
        x0 = RK4(dt, x0, -1.0);
    }
    avg_noise_back /= N;

    x0 = TX_C;
    float avg_noise_forward = 0.0;
    for(int i = 0; i < N; i++)
    {
        x0 = RK4(dt, x0, 1.0);
        avg_noise_forward += texture(noise, x0).x;
    }
    avg_noise_forward /= N;

    color = vec4((((avg_noise_back + avg_noise_forward) * 0.5 - 0.5) * 4.0) + 0.5);
})glsl";