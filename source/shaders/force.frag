#pragma once

inline constexpr char force_frag[] = R"glsl(
#version 430 core
#line 5

uniform vec2 pos;
layout(binding = 0) uniform sampler2D velocity;

uniform vec2 force;
uniform float dt;

in vec2 TX_C;

out vec4 new_velocity;

void main()
{
    new_velocity.xy = texture(velocity, TX_C).xy;

    vec2 d = TX_C - pos;

    float falloff = 0.5 * exp(-dot(d, d) * 1e3);

    new_velocity.xy += dt * falloff * force;
})glsl";