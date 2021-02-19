#include "fluid-simulator.hpp"

#include <exception>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <nanogui/nanogui.h>

#include <nanogui/opengl.h>

#include <glm/gtx/color_space.hpp>

#include "../shaders/screen.vert"
#include "../shaders/draw.frag"
#include "../shaders/advect.frag"
#include "../shaders/force.frag"
#include "../shaders/jacobi-diffusion.frag"
#include "../shaders/jacobi-pressure.frag"
#include "../shaders/divergence.frag"
#include "../shaders/curl.frag"
#include "../shaders/vorticity.frag"
#include "../shaders/gradient-subtract.frag"
#include "../shaders/add-ink.frag"
#include "../shaders/streamlines.frag"

#include "config.hpp"
#include "../gl-util/fbo.hpp"
#include "util.hpp"

FluidSimulator::FluidSimulator(Widget* parent, const std::shared_ptr<Config> &cfg) : 
    Canvas(parent, 1, false), quad(), cfg(cfg),
    draw_shader(screen_vert, draw_frag, "draw"),
    advect_shader(screen_vert, advect_frag, "advect"),
    force_shader(screen_vert, force_frag, "force"),
    jacobi_diffusion_shader(screen_vert, jacobi_diffusion_frag, "jacobi-diffusion"),
    jacobi_pressure_shader(screen_vert, jacobi_pressure_frag, "jacobi-pressure"),
    divergence_shader(screen_vert, divergence_frag, "divergence"),
    curl_shader(screen_vert, curl_frag, "curl"),
    vorticity_shader(screen_vert, vorticity_frag, "vorticity"),
    gradient_subtract_shader(screen_vert, gradient_subtract_frag, "gradient"),
    add_ink_shader(screen_vert, add_ink_frag, "ink"),
    streamlines_shader(screen_vert, streamlines_frag, "streamlines")
{
    resize();
}

void FluidSimulator::draw_contents()
{
    time = glfwGetTime();

    //if (time > last_time) dt = time - last_time;

    last_time = time;

    if (paused)
    {
        return;
    }

    int prev_viewport[4];
    glGetIntegerv(GL_VIEWPORT, prev_viewport);
    glViewport(0, 0, simulation_size.x, simulation_size.y);

    int prev_scissor[4];
    glGetIntegerv(GL_SCISSOR_BOX, prev_scissor);
    glScissor(0, 0, simulation_size.x, simulation_size.y);

    quad.bind();

    applyForce();
    selfAdvectVelocity();
    computeCurl();
    applyVorticityConfinement();
    diffuseVelocity();
    computeDivergence();
    computePressure();
    subtractPressureGradient();

    glViewport(0, 0, fb_size.x, fb_size.y);
    glScissor(0, 0, fb_size.x, fb_size.y);

    if (vis_mode == INK)
        updateInk();
    else
        createStreamlines();

    glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
    glScissor(prev_scissor[0], prev_scissor[1], prev_scissor[2], prev_scissor[3]);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    draw_shader.use();

    glUniform2fv(draw_shader.getLocation("tx_size"), 1, &sim_tx_size[0]);

    if (vis_mode == INK)
        ink->bindTexture(0);
    else
        streamlines->bindTexture(0);

    quad.draw();
    
    if (save_next) saveRender();
}

void FluidSimulator::selfAdvectVelocity()
{
    temp_fbo->bind();

    advect_shader.use();

    glUniform2fv(advect_shader.getLocation("tx_size"), 1, &sim_tx_size[0]);
    glUniform1f(advect_shader.getLocation("inv_dx"), 1.0f / dx);
    glUniform1f(advect_shader.getLocation("dt"), dt);

    velocity->bindTexture(0, GL_LINEAR);
    velocity->bindTexture(1, GL_LINEAR);

    quad.draw();

    std::swap(velocity, temp_fbo);
}

void FluidSimulator::diffuseVelocity()
{
    jacobi_diffusion_shader.use();

    float dx2_nudt = std::powf(dx, 2) / ((float)cfg->nu * dt);

    glUniform2fv(jacobi_diffusion_shader.getLocation("tx_size"), 1, &sim_tx_size[0]);
    glUniform1f(jacobi_diffusion_shader.getLocation("dx2_nudt"), dx2_nudt);

    velocity->bindTexture(1);

    for (int i = 0; i < JACOBI_ITERATIONS; i++)
    {
        temp_fbo->bind();

        velocity->bindTexture(0);

        quad.draw();

        std::swap(velocity, temp_fbo);
    }
    std::swap(velocity, temp_fbo);
}

void FluidSimulator::applyForce()
{
    temp_fbo->bind();

    force_shader.use();

    velocity->bindTexture(0);

    glm::vec2 force = (float)cfg->F * glm::vec2(std::cos(cfg->F_angle), std::sin(cfg->F_angle));

    glUniform2fv(force_shader.getLocation("tx_size"), 1, &sim_tx_size[0]);
    glUniform2fv(force_shader.getLocation("pos"), 1, &mouse_pos[0]);
    glUniform2fv(force_shader.getLocation("force"), 1, &force[0]);
    glUniform1f(force_shader.getLocation("dt"), dt);

    quad.draw();

    std::swap(velocity, temp_fbo);
}

void FluidSimulator::computeCurl()
{
    curl->bind();

    curl_shader.use();

    velocity->bindTexture(0);

    glUniform2fv(curl_shader.getLocation("tx_size"), 1, &sim_tx_size[0]);
    glUniform1f(curl_shader.getLocation("half_inv_dx"), 0.5f / dx);

    quad.draw();
}

void FluidSimulator::applyVorticityConfinement()
{
    temp_fbo->bind();

    vorticity_shader.use();

    velocity->bindTexture(0);
    curl->bindTexture(1);

    glUniform2fv(vorticity_shader.getLocation("tx_size"), 1, &sim_tx_size[0]);
    glUniform1f(vorticity_shader.getLocation("half_inv_dx"), 0.5f / dx);
    glUniform1f(vorticity_shader.getLocation("dt"), dt);
    glUniform1f(vorticity_shader.getLocation("vorticity_scale"), cfg->vorticity);

    quad.draw();

    std::swap(temp_fbo, velocity);
}

void FluidSimulator::computeDivergence()
{
    divergence->bind();

    divergence_shader.use();

    velocity->bindTexture(0);

    glUniform2fv(divergence_shader.getLocation("tx_size"), 1, &sim_tx_size[0]);
    glUniform1f(divergence_shader.getLocation("half_inv_dx"), 0.5f / dx);

    quad.draw();
}

void FluidSimulator::computePressure()
{
    jacobi_pressure_shader.use();

    glUniform2fv(jacobi_pressure_shader.getLocation("tx_size"), 1, &sim_tx_size[0]);
    glUniform1f(jacobi_pressure_shader.getLocation("dx2"), std::pow(dx, 2));

    divergence->bindTexture(1);

    for (int i = 0; i < JACOBI_ITERATIONS; i++)
    {
        temp_fbo->bind();

        pressure->bindTexture(0);        

        quad.draw();

        std::swap(temp_fbo, pressure);
    }
    std::swap(temp_fbo, pressure);
}

void FluidSimulator::subtractPressureGradient()
{
    temp_fbo->bind();

    gradient_subtract_shader.use();

    pressure->bindTexture(0);
    velocity->bindTexture(1);

    glUniform2fv(gradient_subtract_shader.getLocation("tx_size"), 1, &sim_tx_size[0]);
    glUniform1f(gradient_subtract_shader.getLocation("half_inv_dx"), 0.5f / dx);

    quad.draw();

    std::swap(temp_fbo, velocity);
}

void FluidSimulator::updateInk()
{
    temp_large->bind();
    add_ink_shader.use();
    ink->bindTexture(0);
    //glm::vec3 color = glm::rgbColor(glm::vec3(std::fmod(time * 10.0, 360.0f), 0.6f, /*0.75f + 0.25f * */std::sin(0.5f * time)));
    //glm::vec3 color = glm::rgbColor(glm::vec3(std::fmod(time * 10.0, 360.0f), 0.5f, 1.0));
    static float time_ = 0.0;
    time_ += dt;
    glm::vec3 color = glm::rgbColor(glm::vec3(std::fmod(40.0f + time_, 360.0f), 0.5f, std::cosf(time_ * 0.05f)));
    glUniform2fv(add_ink_shader.getLocation("tx_size"), 1, &sim_tx_size[0]);
    glUniform3fv(add_ink_shader.getLocation("color"), 1, &color[0]);
    glUniform2fv(add_ink_shader.getLocation("pos"), 1, &mouse_pos[0]);
    glUniform1f(add_ink_shader.getLocation("time"), time_);
    glUniform1f(add_ink_shader.getLocation("dt"), dt);
    quad.draw();
    std::swap(ink, temp_large);

    temp_large->bind();
    advect_shader.use();
    glUniform2fv(advect_shader.getLocation("tx_size"), 1, &sim_tx_size[0]);
    glUniform1f(advect_shader.getLocation("inv_dx"), 1.0f / dx);
    glUniform1f(advect_shader.getLocation("dt"), dt);

    velocity->bindTexture(0, GL_LINEAR);
    ink->bindTexture(1, GL_LINEAR);

    quad.draw();
    std::swap(ink, temp_large);
}

void FluidSimulator::createStreamlines()
{
    streamlines->bind();

    streamlines_shader.use();

    glUniform2fv(streamlines_shader.getLocation("tx_size"), 1, &sim_tx_size[0]);
    glUniform1f(streamlines_shader.getLocation("inv_dx"), 1.0f / dx);

    velocity->bindTexture(0, GL_LINEAR);
    noise->bindTexture(1, GL_LINEAR);

    quad.draw();
}

bool FluidSimulator::mouse_drag_event(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers)
{
    click = false;

    auto px = p - position();
    px.y() = size().y() - px.y();
    mouse_pos.x = px.x() / (float)size().x();
    mouse_pos.y = px.y() / (float)size().y();

    return false;
}

bool FluidSimulator::mouse_button_event(const nanogui::Vector2i &p, int button, bool down, int modifiers)
{
    mouse_active = down;

    click = mouse_active;

    return false;
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

    simulation_size = fb_size / 4;
    sim_tx_size = 1.0f / glm::vec2(simulation_size);

    for (auto& s_fbo : { &temp_fbo, &velocity, &pressure, &divergence, &curl })
    {
        *s_fbo = std::make_unique<FBO>(simulation_size);
    }

    //std::vector<glm::vec4> ink_d(fb_size.x * fb_size.y, glm::vec4(glm::rgbColor(glm::vec3(200.0f, 0.5f, 0.5f)), 1.0));
    //ink = std::make_unique<FBO>(fb_size, 0.5f, ink_d.data());
    ink = std::make_unique<FBO>(fb_size, 0.5f);
    streamlines = std::make_unique<FBO>(fb_size);
    temp_large = std::make_unique<FBO>(fb_size);

    // create noise texture
    auto noise_size = fb_size;
    std::vector<glm::vec4> initial_noise(noise_size.x * noise_size.y, glm::vec4(0.0f));
    for (auto& n : initial_noise)
    {
        n = glm::vec4(((float)rand() / (RAND_MAX)));
    }
    noise = std::make_unique<FBO>(noise_size, 0.0f, initial_noise.data());
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