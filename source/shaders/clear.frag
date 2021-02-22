#pragma once

inline constexpr char clear_frag[] = R"glsl(
#version 430 core
#line 5

uniform vec4 clear_color;

out vec4 color;

void main()
{
    color = clear_color;
})glsl";