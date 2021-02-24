#pragma once

inline constexpr char streamlines_frag[] = R"glsl(
#version 430 core
#line 5

uniform vec2 world2tx;
uniform int N;
uniform float trace_time;

layout(binding = 0) uniform sampler2D velocity;
layout(binding = 1) uniform sampler2D noise;

in vec2 TX_C;

out vec4 color;

vec2 v(vec2 tx)
{    
    vec2 vel = texture(velocity, tx).xy;

    if(tx.x < 0.0 || tx.x > 1.0) vel.x = -vel.x;
    if(tx.y < 0.0 || tx.y > 1.0) vel.y = -vel.y;

    return vel;
}

vec2 RK4(float dt, vec2 x)
{
    vec2 k1 = v(x);
    vec2 k2 = v(x + world2tx * 0.5 * k1 * dt);
    vec2 k3 = v(x + world2tx * 0.5 * k2 * dt);
    vec2 k4 = v(x + world2tx * k3 * dt);
    return x + world2tx * dt * (k1 + 2.0 * (k2 + k3) + k4) / 6.0;
}

void main()
{
    float dt = trace_time / N;

    float avg_noise = texture(noise, TX_C).x;

    // Forward integrate
    vec2 x = TX_C;
    for(int i = 0; i < N; i++)
    {
        x = RK4(dt, x);
        avg_noise += texture(noise, x).x;
    }

    // Backward integrate
    x = TX_C;
    for(int i = 0; i < N; i++)
    {
        x = RK4(-dt, x);
        avg_noise += texture(noise, x).x;
    }

    avg_noise /= (2.0 * N + 1.0);

    // Apply contrast
    const float contrast = 4.0;
    avg_noise = ((avg_noise - 0.5) * contrast) + 0.5;

    color = vec4(clamp(avg_noise, 0.0, 1.0));
})glsl";