#include "fbo.hpp"

#include <exception>
#include <stdexcept>
#include <vector>

#include <nanogui/opengl.h>

FBO::FBO(const glm::ivec2 &size, float initial, void* initial_data) : size(size)
{
    glGenFramebuffers(1, &handle);
    glBindFramebuffer(GL_FRAMEBUFFER, handle);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // initial state
    if (initial_data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, initial_data);
    }
    else
    {
        std::vector<float> data(size.x * size.y * 4, initial);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, data.data());
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("Framebuffer not complete.");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FBO::~FBO()
{
    glDeleteTextures(1, &texture);
    glDeleteFramebuffers(1, &handle);
}

void FBO::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
}

void FBO::bindTexture(int binding, int interpolation)
{
    glActiveTexture(GL_TEXTURE0 + binding);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolation);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolation);
}