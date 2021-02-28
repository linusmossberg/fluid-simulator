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

in vec2 C, L, R, T, B;

#define c(TX) texture(curl, TX).x

void main()
{
    vec2 force = half_inv_dx * vec2(abs(c(T)) - abs(c(B)), abs(c(R)) - abs(c(L)));
    float length2 = max(1e-6, dot(force, force));
    force *= inversesqrt(length2) * vorticity_scale * c(C) * vec2(1, -1);
    new_velocity.xy = texture(velocity, C).xy + force * dt;
})glsl";