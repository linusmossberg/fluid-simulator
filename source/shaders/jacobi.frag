#pragma once

inline constexpr char jacobi_frag[] = R"glsl(
#version 430 core
#line 5

uniform vec2 texel_size;
uniform float alpha;
uniform float beta;

// Ax = b
layout(binding = 0) uniform sampler2D x_tex;
layout(binding = 1) uniform sampler2D b_tex;

in vec2 interpolated_texcoord;

out vec4 color;

vec4 x(float x_offset, float y_offset)
{
    return texture(x_tex, (interpolated_texcoord + vec2(x_offset, y_offset) * texel_size));
}

void main()
{
    vec4 b = texture(b_tex, interpolated_texcoord);
    color = (x(-1, 0) + x(0, 1) + x(1, 0) + x(0, -1) + alpha * b) / beta;
})glsl";