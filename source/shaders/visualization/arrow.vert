#pragma once

inline constexpr char arrow_vert[] = R"glsl(
#version 430 core
#line 5

layout(binding = 0) uniform sampler2D velocity;

layout(location = 0) in vec2 position;

uniform vec2 coord;
uniform vec2 scale;

void main()
{
    vec2 v = texture(velocity, coord).xy;
    vec2 d = v * inversesqrt(max(1e-3, dot(v,v)));
    mat2 rot = mat2(d.x, d.y, -d.y, d.x);
    gl_Position = vec4((coord - 0.5) * 2.0 + rot * position * scale, 0.0, 1.0);
})glsl";