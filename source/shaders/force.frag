#pragma once

inline constexpr char force_frag[] = R"glsl(
#version 430 core
#line 5

uniform vec2 pos;
layout(binding = 0) uniform sampler2D velocity;

uniform vec2 force;
uniform float dt;

in vec2 interpolated_texcoord;

out vec4 new_velocity;

void main()
{
    const float radius = 0.025;

    new_velocity.xy = texture(velocity, interpolated_texcoord).xy;

    vec2 d = interpolated_texcoord - pos;

    float falloff = 0.5 * exp(-dot(d, d) * 1e3);

    new_velocity.xy += dt * falloff * force;

    // vec2 dir = interpolated_texcoord - pos;
    // float falloff = exp(-dot(dir,dir) * 1e3);
    // vec2 v = texture(velocity, interpolated_texcoord).xy;
    // new_velocity.xy = v + normalize(dir) * falloff;
})glsl";