#include "arrow.hpp"

#include <nanogui/opengl.h>

Arrow::~Arrow()
{
    if (initiated)
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
}

void Arrow::init()
{
    if (!initiated)
    {
        constexpr float vertices[] =
        {
            0.000f,  0.048f,
            0.847f,  0.048f,
            0.761f,  0.132f,
            0.761f,  0.237f,
            1.000f,  0.000f,
            0.000f, -0.048f,
            0.761f, -0.237f,
            0.761f, -0.132f,
            0.847f, -0.048f
        };

        constexpr unsigned int indices[] =
        {
            0, 8, 1,
            1, 8, 4,
            8, 6, 4,
            1, 3, 2,
            0, 5, 8,
            8, 7, 6,
            1, 4, 3
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        initiated = true;
    }
}

void Arrow::bind()
{
    if (!initiated) init();

    glBindVertexArray(VAO);
}

void Arrow::draw()
{
    glDrawElements(GL_TRIANGLES, 21, GL_UNSIGNED_INT, 0);
}