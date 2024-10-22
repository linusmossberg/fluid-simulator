#pragma once

#include <string>

class Shader
{
public:
    Shader(const char* vert_source, const char* frag_source, std::string name = "");

    ~Shader();

    int getLocation(const char* name) const;

    void use() const;

    int handle;
};