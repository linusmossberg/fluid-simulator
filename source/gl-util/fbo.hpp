#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <nanogui/opengl.h>

class FBO
{
public:
    FBO(const glm::ivec2 &size, float initial = 0.0f, void* initial_data = nullptr);
    ~FBO();

    void bind() const;

    void bindTexture(int binding, int interpolation = GL_NEAREST) const;

    std::pair<glm::vec4, glm::vec4> minMax() const;

    unsigned int handle, texture;
    const glm::ivec2 size;
};