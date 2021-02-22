#pragma once

class Quad
{
public:
    ~Quad();

    static void bind();
    static void draw();

private:
    static void init();
    inline static unsigned int VBO, VAO;
    inline static bool initiated = false;
};