#pragma once

inline constexpr char velocity_boundary_frag[] = R"glsl(
#version 430 core
#line 5

layout(binding = 0) uniform sampler2D velocity;
layout(binding = 1) uniform sampler2D boundary;

ivec2 px = ivec2(gl_FragCoord.xy);

#define b(X, Y) texelFetch(boundary, px + ivec2(X, Y), 0).x

out vec4 new_velocity;

//#define v(X, Y) texelFetch(velocity, px + ivec2(X, Y), 0).xy

void main()
{
    ivec2 mx = textureSize(velocity, 0) - 1;

    vec2 v = texelFetch(velocity, px, 0).xy;
    if(px.x <= 0) { v.x = -texelFetch(velocity, px + ivec2(1, 0), 0).x; }
    if(px.x >= mx.x) { v.x = -texelFetch(velocity, px + ivec2(-1, 0), 0).x; }
    if(px.y <= 0) { v.y = -texelFetch(velocity, px + ivec2(0, 1), 0).y; }
    if(px.y >= mx.y) { v.y = -texelFetch(velocity, px + ivec2(0, -1), 0).y; }
    new_velocity.xy = v;

    // new_velocity.xy = texelFetch(velocity, px, 0).xy;

    // // not boundary texel
    // if(b(0, 0) > 0.5)
    // {
    //     return;
    // }

    // float bL = b(-1,  0);
    // float bR = b( 1,  0);
    // float bB = b( 0, -1);
    // float bT = b( 0,  1);

    // vec2 vL = v(-1,  0);
    // vec2 vR = v( 1,  0);
    // vec2 vB = v( 0, -1);
    // vec2 vT = v( 0,  1);

    // new_velocity.xy = -vec2(bL * vL.x + bR * vR.x, bB * vB.y + bT * vT.y);

    // new_velocity.xy = texelFetch(velocity, px, 0).xy;

    // if(b(0, 0) > 0.5)
    // {
    //     return;
    // }

    // float bL = b(-1,  0);
    // float bR = b( 1,  0);
    // float bB = b( 0, -1);
    // float bT = b( 0,  1);

    // if(bL + bR + bB + bT < 0.5)
    // {
    //     new_velocity.xy = vec2(0.0);
    //     return;
    // }

    // vec2 vL = v(-1,  0);
    // vec2 vR = v( 1,  0);
    // vec2 vB = v( 0, -1);
    // vec2 vT = v( 0,  1);

    // if(bL > 0.5)
    // {
    //     new_velocity.x = -vL.x;
    // }
    // if(bR > 0.5)
    // {
    //     new_velocity.x = -vR.x;
    // }
    // if(bB > 0.5)
    // {
    //     new_velocity.y = -vB.y;
    // }
    // if(bT > 0.5)
    // {
    //     new_velocity.y = -vT.y;
    // }
})glsl";