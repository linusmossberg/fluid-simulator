#pragma once

inline constexpr char ink_image_frag[] = R"glsl(
#version 430 core
#line 5

layout(binding = 0) uniform sampler2D image;

in vec2 TX_C;

out vec4 ink;

void main()
{
    ink = texture(image, TX_C);
})glsl";