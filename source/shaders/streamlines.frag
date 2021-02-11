#pragma once

inline constexpr char streamlines_frag[] = R"glsl(
#version 430 core
#line 5

uniform float inv_dx;

layout(binding = 0) uniform sampler2D velocity;
layout(binding = 1) uniform sampler2D noise;

in vec2 interpolated_texcoord;

out vec4 color;

#define v(X) texture(velocity, interpolated_texcoord + X).xy

vec2 RK4(float dt, vec2 x0)
{
    vec2 k1 = texture(velocity, x0).xy;
    vec2 k2 = v(-0.5 * k1 * dt);
    vec2 k3 = v(-0.5 * k2 * dt);
    vec2 k4 = v(-k3 * dt);
    return x0 - inv_dx * dt * (k1 + 2.0 * (k2 + k3) + k4) / 6.0;
}

float srgbGammaCompress(float c)
{
    return mix(1.055 * pow(c, 1.0/2.4) - 0.055, c * 12.92, c < 0.0031308);
}

void main()
{
    vec2 x0 = interpolated_texcoord;

    const int N = 50;
    const float trace_time = 0.5;
    const float dt = trace_time / N;

    float avg_noise = 0.0;
    for(int i = 0; i < N; i++)
    {
        avg_noise += texture(noise, x0).x;
        x0 = RK4(dt, x0);
    }
    avg_noise /= N;

    color = vec4(((avg_noise - 0.5) * 4.0) + 0.5);
})glsl";