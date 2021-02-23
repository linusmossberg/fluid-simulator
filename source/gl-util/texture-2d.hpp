#pragma once

#include <filesystem>

#include <glm/glm.hpp>

class Texture2D
{
public:
    Texture2D();
    Texture2D(const std::filesystem::path& path);

    ~Texture2D();

    void loadImage(const std::filesystem::path& path) const;
    void setData(const glm::ivec2& size, const void* data) const;

    void bind(int binding) const;

    unsigned int handle;
};
