#pragma once

inline constexpr char draw_frag[] = R"glsl(
#version 430 core
#line 5

in vec2 C;

layout(binding = 0) uniform sampler2D image;

out vec4 color;

uniform float exposure;

vec3 srgbGammaCompress(vec3 c)
{
    return mix(
        1.055 * pow(c, vec3(1.0/2.4)) - 0.055,
        c * 12.92,
        lessThan(c, vec3(0.0031308))
    );
}

vec3 srgbGammaExpand(vec3 c)
{
    return mix(
        pow((c + 0.055) / 1.055, vec3(2.4)), 
        c / 12.92,
        lessThan(c, vec3(0.04045))
    );
}

void main()
{
    color.xyz = srgbGammaCompress(exposure * srgbGammaExpand(texture(image, C).xyz));
})glsl";