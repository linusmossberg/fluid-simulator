#pragma once

inline constexpr char color_map_frag[] = R"glsl(
#version 430 core
#line 5

uniform vec2 min_max;

layout(binding = 0) uniform sampler2D scalar_field;
layout(binding = 1) uniform sampler1D transfer_function;

in vec2 TX_C;

out vec4 map;

void main()
{
    float s = texture(scalar_field, TX_C).x;
    map = texture(transfer_function, (s - min_max.x) / (min_max.y - min_max.x));
})glsl";