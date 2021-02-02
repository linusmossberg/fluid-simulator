#pragma once

inline constexpr char force_frag[] = R"glsl(
#version 430 core
#line 5

uniform vec2 pos;
layout(binding = 0) uniform sampler2D velocity;

in vec2 interpolated_texcoord;

out vec4 new_velocity;

void main()
{
    vec2 dir = interpolated_texcoord - pos;
    float falloff = exp(-dot(dir,dir) / 0.01);
    vec2 v = texture(velocity, interpolated_texcoord).xy;
    new_velocity.xy = v + normalize(dir) * falloff;
    new_velocity.zw = vec2(0);
})glsl";