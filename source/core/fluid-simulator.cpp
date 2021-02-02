#include "fluid-simulator.hpp"

#include <exception>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <nanogui/nanogui.h>

#include <nanogui/opengl.h>

#include "../shaders/screen.vert"
#include "../shaders/draw.frag"
#include "../shaders/advect.frag"
#include "../shaders/force.frag"
#include "../shaders/jacobi.frag"
#include "../shaders/divergence.frag"
#include "../shaders/gradient-subtract.frag"
#include "../shaders/add-ink.frag"

#include "config.hpp"
#include "../gl-util/fbo.hpp"
#include "util.hpp"

FluidSimulator::FluidSimulator(Widget* parent, const std::shared_ptr<Config> &cfg) : 
    Canvas(parent, 1, false), quad(), cfg(cfg),
    draw_shader(screen_vert, draw_frag, "draw"),
    advect_shader(screen_vert, advect_frag, "advect"),
    force_shader(screen_vert, force_frag, "force"),
    jacobi_shader(screen_vert, jacobi_frag, "jacobi"),
    divergence_shader(screen_vert, divergence_frag, "divergence"),
    gradient_subtract_shader(screen_vert, gradient_subtract_frag, "gradient"),
    add_ink_shader(screen_vert, add_ink_frag)
{
    resize();
}

void FluidSimulator::draw_contents()
{
    time = glfwGetTime();

    if (time < last_time)
        dt = last_time - time;
    else
        dt = 1.0f / 60.0f;

    last_time = time;

    quad.bind();

    selfAdvectVelocity();
    diffuseVelocity();
    applyForce();
    computeDivergence();
    computePressure();
    subtractPressureGradient();
    updateInk();

    // draw
    draw_shader.use();
    
    ink->bindTexture(0);

    quad.draw();

    if (save_next) saveRender();
}

void FluidSimulator::selfAdvectVelocity()
{
    fbo0->bind();

    advect_shader.use();

    glUniform1f(advect_shader.getLocation("dx"), dx);
    glUniform1f(advect_shader.getLocation("dt"), dt);

    velocity->bindTexture(0);
    velocity->bindTexture(1);

    quad.draw();

    fbo0->unBind();

    std::swap(velocity, fbo0);
}

void FluidSimulator::diffuseVelocity()
{
    float alpha_diffusion = std::pow(dx, 2) / ((float)cfg->nu * dt);
    float beta_diffusion = 4 + alpha_diffusion;

    jacobi_shader.use();
    glUniform2fv(jacobi_shader.getLocation("texel_size"), 1, &texel_size[0]);
    glUniform1f(jacobi_shader.getLocation("alpha"), alpha_diffusion);
    glUniform1f(jacobi_shader.getLocation("beta"), beta_diffusion);

    for (int i = 0; i < JACOBI_ITERATIONS; i++)
    {
        fbo0->bind();

        velocity->bindTexture(0);
        velocity->bindTexture(1);

        quad.draw();

        fbo0->unBind();

        std::swap(velocity, fbo0);
    }
    std::swap(velocity, fbo0);
}

void FluidSimulator::applyForce()
{
    if (mouse_active)
    {
        fbo0->bind();

        force_shader.use();

        glm::vec2 pos = glm::clamp(glm::vec2(mouse_pos) / glm::vec2(fb_size), glm::vec2(0.0), glm::vec2(1.0));

        velocity->bindTexture(0);

        glUniform2fv(force_shader.getLocation("pos"), 1, &pos[0]);

        quad.draw();

        fbo0->unBind();

        std::swap(velocity, fbo0);
    }
}

void FluidSimulator::computeDivergence()
{
    divergence->bind();

    divergence_shader.use();

    velocity->bindTexture(0);

    glUniform1f(divergence_shader.getLocation("dx"), dx);
    glUniform2fv(divergence_shader.getLocation("texel_size"), 1, &texel_size[0]);

    quad.draw();

    divergence->unBind();
}

void FluidSimulator::computePressure()
{
    float alpha_pressure = -std::pow(dx, 2);
    float beta_pressure = 4;

    jacobi_shader.use();

    glUniform2fv(jacobi_shader.getLocation("texel_size"), 1, &texel_size[0]);
    glUniform1f(jacobi_shader.getLocation("alpha"), alpha_pressure);
    glUniform1f(jacobi_shader.getLocation("beta"), beta_pressure);

    divergence->bindTexture(1);

    for (int i = 0; i < JACOBI_ITERATIONS; i++)
    {
        fbo0->bind();

        pressure->bindTexture(0);        

        quad.draw();

        fbo0->unBind();

        std::swap(fbo0, pressure);
    }
    std::swap(fbo0, pressure);
}

void FluidSimulator::subtractPressureGradient()
{
    fbo0->bind();

    gradient_subtract_shader.use();

    pressure->bindTexture(0);
    velocity->bindTexture(1);

    glUniform1f(gradient_subtract_shader.getLocation("dx"), dx);
    glUniform2fv(gradient_subtract_shader.getLocation("texel_size"), 1, &texel_size[0]);

    quad.draw();

    fbo0->unBind();

    std::swap(fbo0, velocity);
}

void FluidSimulator::updateInk()
{
    fbo0->bind();
    add_ink_shader.use();
    glm::vec2 pos(0.5);
    ink->bindTexture(0);
    glUniform2fv(add_ink_shader.getLocation("pos"), 1, &pos[0]);
    glUniform1f(add_ink_shader.getLocation("time"), time);
    quad.draw();
    fbo0->unBind();
    std::swap(ink, fbo0);

    fbo0->bind();
    advect_shader.use();
    glUniform1f(advect_shader.getLocation("dx"), dx);
    glUniform1f(advect_shader.getLocation("dt"), dt);

    velocity->bindTexture(0);
    ink->bindTexture(1);

    quad.draw();
    fbo0->unBind();
    std::swap(ink, fbo0);
}

bool FluidSimulator::mouse_drag_event(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers)
{
    mouse_pos = {p.x(), fb_size.y - p.y()};
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

    texel_size = 1.0f / glm::vec2(fb_size);

    fbo0 = std::make_unique<FBO>(fb_size);
    velocity = std::make_unique<FBO>(fb_size);
    pressure = std::make_unique<FBO>(fb_size);
    divergence = std::make_unique<FBO>(fb_size);
    ink = std::make_unique<FBO>(fb_size);
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