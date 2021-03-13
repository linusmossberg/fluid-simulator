#include "fbo.hpp"

#include <exception>
#include <stdexcept>
#include <vector>

#include <nanogui/opengl.h>

#include "shader.hpp"
#include "quad.hpp"

#include "../shaders/screen.vert"
#include "../shaders/clear.frag"

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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

void FBO::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
}

void FBO::bindTexture(int binding, int interpolation) const
{
    glActiveTexture(GL_TEXTURE0 + binding);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolation);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolation);
}

std::pair<glm::vec4, glm::vec4> FBO::minMax() const
{
    glBindTexture(GL_TEXTURE_2D, texture);

    std::vector<glm::vec4> data(size.x * size.y, glm::vec4(0.0));
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data.data());

    glm::vec4 min_v(std::numeric_limits<float>::max());
    glm::vec4 max_v(std::numeric_limits<float>::lowest());

    for (int y = 0; y < size.y; y++)
    {
        for (int x = 0; x < size.x; x++)
        {
            const auto& p = data[((size_t)size.y - 1 - y) * size.x + x];

            for (int c = 0; c < 4; c++)
            {
                if (p[c] < min_v[c]) min_v[c] = p[c];
                if (p[c] > max_v[c]) max_v[c] = p[c];
            }
        }
    }

    return { min_v, max_v };
}

void FBO::clear(const glm::vec4& clear_color) const
{
    static const Shader clear_shader(screen_vert, clear_frag, "clear");

    Quad::bind();

    glViewport(0, 0, size.x, size.y);
    glScissor(0, 0, size.x, size.y);

    bind();
    clear_shader.use();
    glUniform4fv(clear_shader.getLocation("clear_color"), 1, &clear_color[0]);
    Quad::draw();
}