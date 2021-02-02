#pragma once

inline constexpr char advect_frag[] = R"glsl(
#version 430 core
#line 5

uniform float dt;
uniform float dx;

layout(binding = 0) uniform sampler2D velocity;
layout(binding = 1) uniform sampler2D quantity;

in vec2 interpolated_texcoord;

out vec4 color;

void main()
{
    vec2 x1 = interpolated_texcoord;
    vec2 v = texture(velocity, interpolated_texcoord).xy;

    // Backward integrate to previous position using Euler
    vec2 x0 = x1 - (dt / dx) * v;

    color = texture(quantity, x0);
})glsl";