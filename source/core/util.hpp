#pragma once

#include <string>
#include <sstream>

#include <glm/glm.hpp>

inline std::ostream& operator<<(std::ostream& out, const glm::dvec3& v)
{
    return out << std::string("( " + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + " )");
}

inline glm::vec3 hexToColor(const char* hex_string)
{
    std::stringstream ss;
    ss << std::hex << hex_string;

    uint32_t i;
    ss >> i;

    return glm::dvec3((i >> 16) & 0xFF, (i >> 8) & 0xFF, i & 0xFF) / 255.0;
}