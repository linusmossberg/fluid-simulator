#pragma once

inline constexpr char arrow_frag[] = R"glsl(
#version 430 core
#line 5

uniform vec4 arrow_color;

out vec4 color;

void main()
{
    color = arrow_color;
})glsl";