#pragma once

inline constexpr char add_ink_frag[] = R"glsl(
#version 430 core
#line 5

uniform vec2 pos;
uniform float time;

layout(binding = 0) uniform sampler2D ink;

in vec2 interpolated_texcoord;

out vec4 new_ink;

void main()
{
    vec3 color = vec3(0.5 * (1.0 + sin(time * 0.1)), 0.5 * (1.0 + sin(time * 0.2)), 0.5 * (1.0 + sin(time * 0.3)));

    vec2 dir = interpolated_texcoord - pos;
    float amount = 1.0 * exp(-dot(dir,dir) / 0.0001);
    vec4 old_ink = texture(ink, interpolated_texcoord);
    new_ink = old_ink + amount * vec4(color, 1.0);
})glsl";