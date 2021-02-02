#pragma once

inline constexpr char divergence_frag[] = R"glsl(
#version 430 core
#line 5

uniform vec2 texel_size;
uniform float dx;

layout(binding = 0) uniform sampler2D velocity;

in vec2 interpolated_texcoord;

out vec4 divergence;

vec2 v(float x_offset, float y_offset)
{
    return texture(velocity, (interpolated_texcoord + vec2(x_offset, y_offset) * texel_size)).xy;
}

void main()
{
    divergence.x = (0.5 / dx) * (v(1, 0).x - v(-1, 0).x + v(0, 1).y - v(0, -1).y);
    divergence.yzw = vec3(0);
})glsl";