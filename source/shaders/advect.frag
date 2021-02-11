#pragma once

inline constexpr char advect_frag[] = R"glsl(
#version 430 core
#line 5

uniform float dt;
uniform float inv_dx;

layout(binding = 0) uniform sampler2D velocity;
layout(binding = 1) uniform sampler2D quantity;

in vec2 interpolated_texcoord;

out vec4 color;

#define v(X) texture(velocity, interpolated_texcoord + X).xy

vec2 RK4()
{
    vec2 k1 = texelFetch(velocity, ivec2(gl_FragCoord.xy) / (textureSize(quantity, 0) / textureSize(velocity, 0)), 0).xy;
    vec2 k2 = v(-0.5 * k1 * dt);
    vec2 k3 = v(-0.5 * k2 * dt);
    vec2 k4 = v(-k3 * dt);
    return dt * (k1 + 2.0 * (k2 + k3) + k4) / 6.0;
}

void main()
{
    vec2 x1 = interpolated_texcoord;

    //vec2 k1 = texelFetch(velocity, ivec2(gl_FragCoord.xy) / (textureSize(quantity, 0) / textureSize(velocity, 0)), 0).xy;
    //vec2 k2 = texture(velocity, )

    // Backward integrate to previous position using Euler
    vec2 x0 = x1 - inv_dx * RK4();

    color = texture(quantity, x0);

    //color = texture(quantity, x0) * texture(boundary, x0);
})glsl";