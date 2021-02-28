#pragma once

inline constexpr char stencil_vert[] = R"glsl(
#version 430 core
#line 5

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texcoord;

uniform vec2 tx_size;


out vec2    T,
         L, C, R,
            B;

void main()
{
    C = texcoord;
    L = texcoord - vec2(tx_size.x, 0.0);
    R = texcoord + vec2(tx_size.x, 0.0);
    B = texcoord - vec2(0.0, tx_size.y);
    T = texcoord + vec2(0.0, tx_size.y);

    gl_Position = vec4(position, 0.0, 1.0);
})glsl";