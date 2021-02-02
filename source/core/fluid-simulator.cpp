#include "fluid-simulator.hpp"

#include <exception>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <nanogui/nanogui.h>

#include <nanogui/opengl.h>

#include "../shaders/screen.vert"
#include "../shaders/draw.frag"

#include "config.hpp"
#include "../gl-util/fbo.hpp"
#include "util.hpp"

FluidSimulator::FluidSimulator(Widget* parent, const std::shared_ptr<Config> &cfg) : 
    Canvas(parent, 1, false), quad(), cfg(cfg),
    draw_shader(screen_vert, draw_frag)
{
    resize();
}

void FluidSimulator::draw_contents()
{
    draw_shader.use();

    quad.bind();

    glUniform1f(draw_shader.getLocation("nu"), cfg->nu);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    quad.draw();

    if (save_next) saveRender();
}

bool FluidSimulator::mouse_drag_event(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers)
{
    mouse_pos = {p.x(), p.y()};
    click = false;
    return true;
}

bool FluidSimulator::mouse_button_event(const nanogui::Vector2i &p, int button, bool down, int modifiers)
{
    mouse_active = down;

    if (mouse_active)
        glfwSetInputMode(screen()->glfw_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    else
        glfwSetInputMode(screen()->glfw_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    click = mouse_active;

    return true;
}

void FluidSimulator::resize()
{
    fb_size = { cfg->width, cfg->height };
    set_fixed_size({ fb_size.x, fb_size.y });

    if (draw_border())
    {
        fb_size -= 2;
    }
    fb_size = glm::ivec2(glm::vec2(fb_size) * screen()->pixel_ratio());

    fbo0 = std::make_unique<FBO>(fb_size);
    fbo1 = std::make_unique<FBO>(fb_size);
}

void FluidSimulator::saveNextRender(const std::string &filename)
{
    savename = std::filesystem::path(filename).replace_extension(".tga").string();
    std::cout << savename << std::endl;
    save_next = true;
}

void FluidSimulator::saveRender()
{
    if (!save_next || savename.empty()) return;

    struct HeaderTGA
    {
        HeaderTGA(uint16_t width, uint16_t height)
            : width(width), height(height) {}

    private:
        uint8_t begin[12] = { 0, 0, 2 };
        uint16_t width;
        uint16_t height;
        uint8_t end[2] = { 24, 32 };
    };

    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    // GL_RGBA is required for some reason
    std::vector<glm::u8vec4> data((size_t)vp[2] * vp[3]);
    glReadPixels(vp[0], vp[1], vp[2], vp[3], GL_RGBA, GL_UNSIGNED_BYTE, data.data());

    HeaderTGA header(vp[2], vp[3]);

    std::vector<glm::u8vec3> rearranged((size_t)vp[2] * vp[3]);
    for (int y = 0; y < vp[3]; y++)
    {
        for (int x = 0; x < vp[2]; x++)
        {
            const auto &p = data[((size_t)vp[3] - 1 - y) * vp[2] + x];
            rearranged[y * (size_t)vp[2] + x] = { p.b, p.g, p.r };
        }
    }

    std::ofstream image_file(savename, std::ios::binary);
    image_file.write(reinterpret_cast<char*>(&header), sizeof(header));
    image_file.write(reinterpret_cast<char*>(rearranged.data()), rearranged.size() * 3);
    image_file.close();

    save_next = false;
    savename = "";
}