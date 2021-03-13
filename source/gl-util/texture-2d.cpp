#include "texture-2d.hpp"

#include <nanogui/opengl.h>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture2D::Texture2D()
{
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture2D::Texture2D(const std::filesystem::path& path) 
    : Texture2D()
{
    loadImage(path);
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &handle);
}

void Texture2D::loadImage(const std::filesystem::path& path) const
{
    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    uint8_t* image_data = stbi_load(path.string().c_str(), &width, &height, &channels, 0);

    if (!image_data)
    {
        throw std::runtime_error("Unable to load " + path.string());
    }

    int format, internal_format;
    switch (channels)
    {
    case 1: format = GL_RED;  internal_format = GL_R8; break;
    case 2: format = GL_RG;   internal_format = GL_RG8; break;
    case 3: format = GL_RGB;  internal_format = GL_RGB8; break;
    case 4: format = GL_RGBA; internal_format = GL_RGBA8; break;
    default: stbi_image_free(image_data); throw std::runtime_error("Unable to load " + path.string());
    }

    glBindTexture(GL_TEXTURE_2D, handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image_data);
}

void Texture2D::setData(const glm::ivec2& size, const void* data) const
{
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, data);
}

void Texture2D::bind(int binding) const
{
    glActiveTexture(GL_TEXTURE0 + binding);
    glBindTexture(GL_TEXTURE_2D, handle);
}