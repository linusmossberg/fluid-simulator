#pragma once

inline constexpr char pressure_boundary_frag[] = R"glsl(
#version 430 core
#line 5

layout(binding = 0) uniform sampler2D pressure;
layout(binding = 1) uniform sampler2D boundary;

ivec2 px = ivec2(gl_FragCoord.xy);

#define b(X, Y) texelFetch(boundary, px + ivec2(X, Y), 0).x

out vec4 new_pressure;

#define p(X, Y) texelFetch(pressure, px + ivec2(X, Y), 0).x

void main()
{
    // if(b(0, 0) > 0.5)
    // {
    //     new_velocity.xy = v(0, 0);
    //     return;
    // }

    // float bL = b(-1,  0);
    // float bR = b( 1,  0);
    // float bB = b( 0, -1);
    // float bT = b( 0,  1);

    // float boundary_sum = bL + bR + bB + bT;

    // if(boundary_sum < 0.5)
    // {
    //     new_velocity.xy = v(0, 0);
    //     return;
    // }

    // vec2 vL = v(-1,  0);
    // vec2 vR = v( 1,  0);
    // vec2 vB = v( 0, -1);
    // vec2 vT = v( 0,  1);

    // vec2 boundary_velocity = (bL * vL + bR * vR + bB * vB + bT * vT) / boundary_sum;
    // vec2 boundary_normal = normalize(vec2(bL - bR, bB - bT));

    // new_velocity.xy = boundary_normal * length(boundary_velocity);

    // vec2 boundary_velocity = vec2(bL * vL + bR * vR, bB * vB + bT * vT) / (bL + bR + bB + bT);

    // vec2 boundary_normal = normalize(vec2(bR - bL, bT - bB));

    new_pressure.x = texelFetch(pressure, px, 0).x;

    // not boundary texel
    if(b(0, 0) > 0.5)
    {
        return;
    }

    float bL = b(-1,  0);
    float bR = b( 1,  0);
    float bB = b( 0, -1);
    float bT = b( 0,  1);

    float pL = p(-1,  0);
    float pR = p( 1,  0);
    float pB = p( 0, -1);
    float pT = p( 0,  1);

    new_pressure.x = bL * pL + bR * pR + bB * pB + bT * pT;

    // new_velocity.x *= bL * bR;
    // new_velocity.y *= bB * bT;

    // if(bL < 0.5 || bR < 0.5)
    // {
    //     new_velocity.x *= -1.0;
    // }

    // if(bB < 0.5 || bT < 0.5)
    // {
    //     new_velocity.y *= -1.0;
    // }

    // if(bL < 0.5 || bR < 0.5 || bB < 0.5 || bT < 0.5)
    // {
    //     new_velocity *= -1.0;
    // }
})glsl";