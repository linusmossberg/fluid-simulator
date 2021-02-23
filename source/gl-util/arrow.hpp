#pragma once

class Arrow
{
public:
    ~Arrow();

    static void bind();
    static void draw();

private:
    static void init();
    inline static unsigned int VBO, VAO, EBO;
    inline static bool initiated = false;
};