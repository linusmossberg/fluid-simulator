#pragma once

#include <nanogui/canvas.h>

#include <glm/glm.hpp>

#include "../gl-util/shader.hpp"
#include "../gl-util/quad.hpp"

class FBO;
class Config;

class FluidSimulator : public nanogui::Canvas
{
public:
    FluidSimulator(Widget* parent, const std::shared_ptr<Config>& cfg);

    virtual void draw_contents() override;

    void resize();
    void saveNextRender(const std::string& filename);

    virtual bool mouse_drag_event(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers) override;
    virtual bool mouse_button_event(const nanogui::Vector2i &p, int button, bool down, int modifiers) override;

    glm::ivec2 fb_size;
    glm::ivec2 mouse_pos = glm::ivec2(0);
    glm::vec2 texel_size;

    double time;
    double last_time = std::numeric_limits<double>::max();

    float dt = 1.0f / 60.0f;
    float dx = 1.0f;
    const size_t JACOBI_ITERATIONS = 20;

    bool mouse_active = false;

    // Used to prevent large relative movement the first click
    bool click = false;

private:
    void selfAdvectVelocity();
    void diffuseVelocity();
    void applyForce();
    void computeDivergence();
    void computePressure();
    void subtractPressureGradient();
    void updateInk();

    std::shared_ptr<Config> cfg;
    Shader draw_shader;
    Shader advect_shader;
    Shader force_shader;
    Shader jacobi_shader;
    Shader divergence_shader;
    Shader gradient_subtract_shader;
    Shader add_ink_shader;
    Quad quad;

    std::unique_ptr<FBO> velocity;
    std::unique_ptr<FBO> divergence;
    std::unique_ptr<FBO> pressure;
    std::unique_ptr<FBO> fbo0;
    std::unique_ptr<FBO> ink;

    void saveRender();
    bool save_next = false;
    std::string savename = "";
};
