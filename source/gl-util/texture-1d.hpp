#pragma once

#include <vector>

#include <glm/glm.hpp>

class Texture1D
{
public:
    Texture1D(const std::vector<glm::vec3>& colors);

    ~Texture1D();

    void setColors(const std::vector<glm::vec3>& colors);

    void bind(int binding) const;

    unsigned int handle;
};
