#pragma once

inline constexpr char draw_frag[] = R"glsl(
#version 430 core
#line 5

in vec2 interpolated_texcoord;

layout(binding = 0) uniform sampler2D ink;

out vec4 color;

vec3 srgbGammaCompress(vec3 c)
{
    return mix(
        1.055 * pow(c, vec3(1.0/2.4)) - 0.055,
        c * 12.92,
        lessThan(c, vec3(0.0031308))
    );
}

void main()
{
    color = texture(ink, interpolated_texcoord);
})glsl";