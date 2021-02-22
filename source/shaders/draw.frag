#pragma once

inline constexpr char draw_frag[] = R"glsl(
#version 430 core
#line 5

in vec2 TX_C;

layout(binding = 0) uniform sampler2D ink;

out vec4 color;

// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 ACESFilm(vec3 x)
{
    x = max(vec3(0.0), x);
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), vec3(0.0), vec3(1.0));
}

void main()
{
    // Tonemapping doesn't really make sense since I never consider gamma, but it looks good.
    color.xyz = ACESFilm(texture(ink, TX_C).xyz);
})glsl";