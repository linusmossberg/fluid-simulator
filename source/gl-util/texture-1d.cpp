#include "texture-1d.hpp"

#include <nanogui/opengl.h>

Texture1D::Texture1D(const std::vector<glm::vec3>& colors)
{
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_1D, handle);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB32F, colors.size(), 0, GL_RGB, GL_FLOAT, colors.data());
}

Texture1D::~Texture1D()
{
    glDeleteTextures(1, &handle);
}

void Texture1D::setColors(const std::vector<glm::vec3>& colors)
{
    glBindTexture(GL_TEXTURE_1D, handle);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB32F, colors.size(), 0, GL_RGB, GL_FLOAT, colors.data());
}

void Texture1D::bind(int binding) const
{
    glActiveTexture(GL_TEXTURE0 + binding);
    glBindTexture(GL_TEXTURE_1D, handle);
}