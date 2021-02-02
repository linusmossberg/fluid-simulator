#pragma once

#include <glm/glm.hpp>
#include <vector>

class FBO
{
public:
    FBO(const glm::ivec2 &size);
    ~FBO();

    void bind();

    void unBind();

    void bindTexture(int binding);

    unsigned int handle, texture;
    const glm::ivec2 size;

    int prev_viewport[4] = { 0 };
    int prev_scissor[4] = { 0 };
};