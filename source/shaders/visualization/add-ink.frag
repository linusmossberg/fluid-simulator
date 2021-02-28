#pragma once

inline constexpr char add_ink_frag[] = R"glsl(
#version 430 core
#line 5

uniform vec2 pos;
uniform float dt;
uniform vec3 color;
uniform vec2 tx_size;

layout(binding = 0) uniform sampler2D ink;

in vec2 C;

out vec4 new_ink;

#define PI 3.1415926535897932384626433832795

void main()
{
    vec2 aspect = vec2(1.0, tx_size.x / tx_size.y) * 256.0;

    vec2 d = (C - pos) * aspect;

    const float radius = 2.0;
    const float falloff_integral = PI / radius;

    float falloff = exp(-dot(d,d) / radius);
    vec4 old_ink = texture(ink, C);

    new_ink = (old_ink + dt * (falloff / falloff_integral) * vec4(color, 1.0));
})glsl";