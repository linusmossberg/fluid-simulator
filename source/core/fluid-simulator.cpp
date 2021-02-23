#include "fluid-simulator.hpp"

#include <exception>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <random>

#include <nanogui/nanogui.h>

#include <nanogui/opengl.h>

#include <glm/gtx/color_space.hpp>

#include "../shaders/screen.vert"

#include "../shaders/visualization/draw-tonemap.frag"
#include "../shaders/visualization/draw.frag"
#include "../shaders/visualization/add-ink.frag"
#include "../shaders/visualization/streamlines.frag"
#include "../shaders/visualization/ink-image.frag"
#include "../shaders/visualization/color-map.frag"

#include "color-maps.hpp"

#include "config.hpp"
#include "../gl-util/fbo.hpp"
#include "../gl-util/shader.hpp"
#include "../gl-util/quad.hpp"
#include "util.hpp"

FluidSimulator::FluidSimulator(Widget* parent, const std::shared_ptr<Config>& cfg) :
    Canvas(parent, 1, false), cfg(cfg), fluid_solver(cfg),
    transfer_function(ColorMap::color_maps[0])
{
    resize();
}

void FluidSimulator::draw_contents()
{
    double time = glfwGetTime();

    if (!fixed_dt && time > last_time) cfg->dt = float(time - last_time);

    last_time = time;

    int prev_viewport[4];
    glGetIntegerv(GL_VIEWPORT, prev_viewport);

    int prev_scissor[4];
    glGetIntegerv(GL_SCISSOR_BOX, prev_scissor);

    Quad::bind();

    if (!paused)
    {
        fluid_solver.force_pos = mouse_pos;
        fluid_solver.step();
        sim_time += cfg->dt;
    }

    glViewport(0, 0, fb_size.x, fb_size.y);
    glScissor(0, 0, fb_size.x, fb_size.y);

    if (vis_mode == INK && !paused)
        updateInk();
    else if (vis_mode == STREAMLINES)
        createStreamlines();

    setColorMapRange();

    if (!ink_image_path.empty())
    {
        setInkImage();
    }

    glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
    glScissor(prev_scissor[0], prev_scissor[1], prev_scissor[2], prev_scissor[3]);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    switch (vis_mode)
    {
    case INK:         drawInk(); break;
    case STREAMLINES: drawColorMap(streamlines); break;
    case CURL:        drawColorMap(fluid_solver.curl); break;
    case PRESSURE:    drawColorMap(fluid_solver.pressure); break;
    case SPEED:       drawColorMap(fluid_solver.speed); break;
    case ARROWS:      drawColorMap(fluid_solver.speed); break;
    }
    
    if (save_next) saveRender();
}

void FluidSimulator::updateInk()
{
    static const Shader add_ink_shader(screen_vert, add_ink_frag, "ink");

    if (clear_ink)
    {
        ink->clear(glm::vec4(0.5f));
        clear_ink = false;
    }

    // Add ink
    {
        temp_fbo->bind();
        add_ink_shader.use();
        ink->bindTexture(0);
        float scale = ((std::cosf(sim_time) * 0.5f) + 1.0f) * 500.0f * cfg->ink_rate;
        glm::dvec3 hsv = glm::vec3(std::fmod(sim_time * 10.0f, 360.0f), 0.5f, scale * -std::cosf(sim_time * 0.075f));
        glm::vec3 rgb = glm::rgbColor(hsv);
        glUniform2fv(add_ink_shader.getLocation("tx_size"), 1, &fluid_solver.cell_size[0]);
        glUniform3fv(add_ink_shader.getLocation("color"), 1, &rgb[0]);
        glUniform2fv(add_ink_shader.getLocation("pos"), 1, &mouse_pos[0]);
        glUniform1f(add_ink_shader.getLocation("dt"), cfg->dt);
        Quad::draw();
        std::swap(ink, temp_fbo);
    }

    // Advect ink
    {
        fluid_solver.advect(ink, temp_fbo);
    }
}

void FluidSimulator::createStreamlines()
{
    static const Shader streamlines_shader(screen_vert, streamlines_frag, "streamlines");
    static float last_sim_time = std::numeric_limits<float>::lowest();

    if (last_sim_time != sim_time)
    {
        streamlines->bind();
        streamlines_shader.use();
        glUniform1f(streamlines_shader.getLocation("inv_dx"), 1.0f / fluid_solver.dx);
        fluid_solver.velocity->bindTexture(0, GL_LINEAR);
        noise.bind(1);
        Quad::draw();
    }
    last_sim_time = sim_time;
}

void FluidSimulator::drawColorMap(const std::unique_ptr<FBO>& scalar_field)
{
    static const Shader color_map_shader(screen_vert, color_map_frag, "color-map");

    color_map_shader.use();

    glm::vec2 min_max(cfg->range_min, cfg->range_max);

    glUniform2fv(color_map_shader.getLocation("min_max"), 1, &min_max[0]);

    scalar_field->bindTexture(0, GL_LINEAR);
    transfer_function.bind(1);

    Quad::draw();
}

void FluidSimulator::drawInk()
{
    static const Shader draw_tonemap_shader(screen_vert, draw_tonemap_frag, "draw-tonemap");
    static const Shader draw_shader(screen_vert, draw_frag, "draw");

    if (tonemap)
    {
        draw_tonemap_shader.use();
        glUniform1f(draw_tonemap_shader.getLocation("exposure"), std::powf(2.0f, cfg->ink_exposure));
    }  
    else
    {
        draw_shader.use();
        glUniform1f(draw_shader.getLocation("exposure"), std::powf(2.0f, cfg->ink_exposure));
    }

    ink->bindTexture(0);
    Quad::draw();
}

void FluidSimulator::setColorMap(int index)
{
    transfer_function.setColors(ColorMap::color_maps[index]);
}

void FluidSimulator::setInkImage()
{
    static const Shader ink_image_shader(screen_vert, ink_image_frag, "ink-image");

    if (ink_image_path.empty()) return;

    Texture2D image_texture(ink_image_path);

    ink->bind();
    ink_image_shader.use();
    image_texture.bind(0);
    Quad::draw();

    ink_image_path = "";
}

void FluidSimulator::setColorMapRange()
{
    if (auto_set_range)
    {
        if (vis_mode == STREAMLINES)
        {
            cfg->range_min = 0.0f;
            cfg->range_max = 1.0f;
        }
        else if (vis_mode == CURL)
        {
            auto mm = fluid_solver.curl->minMax();
            float abs_max = std::max(std::abs(mm.first.x), std::abs(mm.second.x));
            cfg->range_min = -abs_max;
            cfg->range_max = abs_max;
        }
        else if (vis_mode == PRESSURE)
        {
            auto mm = fluid_solver.pressure->minMax();
            float abs_max = std::max(std::abs(mm.first.x), std::abs(mm.second.x));
            cfg->range_min = -abs_max;
            cfg->range_max = abs_max;
        }
        else if (vis_mode == SPEED || vis_mode == ARROWS)
        {
            auto mm = fluid_solver.speed->minMax();
            cfg->range_min = mm.first.x;
            cfg->range_max = mm.second.x;
        }
    }
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

    fluid_solver.setSize(glm::vec2(fb_size) / (float)cfg->sim_downscale);

    ink = std::make_unique<FBO>(fb_size, 0.5f);
    streamlines = std::make_unique<FBO>(fb_size);
    temp_fbo = std::make_unique<FBO>(fb_size);

    // create noise texture
    std::mt19937 engine(std::random_device{}());
    std::uniform_real_distribution<float> distribution(0.0, 1.0);
    std::vector<glm::vec4> noise_data(fb_size.x * fb_size.y, glm::vec4(0.0f));
    for (auto& n : noise_data)
    {
        n = glm::vec4(distribution(engine));
    }
    noise.setData(fb_size, noise_data.data());
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