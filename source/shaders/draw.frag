#pragma once

inline constexpr char draw_frag[] = R"glsl(
#version 430 core
#line 5

in vec2 interpolated_texcoord;

layout(binding = 0) uniform sampler2D ink;

out vec4 color;

void main()
{
    color = texture(ink, interpolated_texcoord);
})glsl";