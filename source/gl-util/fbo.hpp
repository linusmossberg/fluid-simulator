#pragma once

#include <glm/glm.hpp>
#include <vector>

class FBO
{
public:
    FBO(const glm::ivec2 &size, float initial = 0.0f);
    ~FBO();

    void bind();

    void bindTexture(int binding);

    unsigned int handle, texture;
    const glm::ivec2 size;
};