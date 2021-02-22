#pragma once

inline constexpr char add_ink_frag[] = R"glsl(
#version 430 core
#line 5

uniform vec2 pos;
uniform float dt;
uniform vec3 color;
uniform vec2 tx_size;

layout(binding = 0) uniform sampler2D ink;

in vec2 TX_C;

out vec4 new_ink;

#define PI 3.1415926535897932384626433832795

void main()
{
    vec2 d = (TX_C - pos) / tx_size;

    const float radius = 2;
    const float falloff_integral = PI / radius;

    float falloff = exp(-dot(d,d) / radius);
    vec4 old_ink = texture(ink, TX_C);

    new_ink = (old_ink + dt * (falloff / falloff_integral) * vec4(color, 1.0));
})glsl";