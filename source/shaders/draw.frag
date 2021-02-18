#pragma once

inline constexpr char draw_frag[] = R"glsl(
#version 430 core
#line 5

in vec2 TX_C;

layout(binding = 0) uniform sampler2D ink;

out vec4 color;

void main()
{
    color = texture(ink, TX_C);
})glsl";