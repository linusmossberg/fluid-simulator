#pragma once

inline constexpr char stencil_vert[] = R"glsl(
#version 430 core
#line 5

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texcoord;

uniform vec2 tx_size;

out vec2 TX_C;
out vec2 TX_L;
out vec2 TX_R;
out vec2 TX_T;
out vec2 TX_B;

void main()
{
    TX_C = texcoord;
    TX_L = texcoord - vec2(tx_size.x, 0.0);
    TX_R = texcoord + vec2(tx_size.x, 0.0);
    TX_B = texcoord - vec2(0.0, tx_size.y);
    TX_T = texcoord + vec2(0.0, tx_size.y);

    gl_Position = vec4(position * 2.0, 0.0, 1.0);
})glsl";