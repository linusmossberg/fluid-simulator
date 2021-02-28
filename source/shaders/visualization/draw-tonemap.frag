#pragma once

inline constexpr char draw_tonemap_frag[] = R"glsl(
#version 430 core
#line 5

in vec2 C;

layout(binding = 0) uniform sampler2D image;

out vec4 color;

uniform float exposure;

// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 ACESFilm(vec3 x)
{
    x = max(vec3(0.0), x);
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), vec3(0.0), vec3(1.0));
}

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
    // Another alternative to gamme expanding is to maintain gamma expanded ink, but ink mixing with 
    // ink that can be both subtractive and additive seems to work better in gamma compressed space.
    color.xyz = srgbGammaCompress(ACESFilm(exposure * srgbGammaExpand(texture(image, C).xyz)));
})glsl";