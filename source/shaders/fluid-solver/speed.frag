#pragma once

inline constexpr char speed_frag[] = R"glsl(
#version 430 core
#line 5

layout(binding = 0) uniform sampler2D velocity;

in vec2 TX_C;

out vec4 speed;

void main()
{
    speed.x = length(texture(velocity, TX_C).xy);
})glsl";