#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <nanogui/opengl.h>

class FBO
{
public:
    FBO(const glm::ivec2 &size, float initial = 0.0f, void* initial_data = nullptr);
    ~FBO();

    void bind();

    void bindTexture(int binding, int interpolation = GL_NEAREST);

    unsigned int handle, texture;
    const glm::ivec2 size;
};