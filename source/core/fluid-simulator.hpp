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
    glm::ivec2 simulation_size;
    glm::vec2 sim_tx_size;
    glm::vec2 mouse_pos = glm::vec2(0.05,0.5);

    double time;
    double last_time = std::numeric_limits<double>::max();

    enum VisMode
    {
        INK,
        STREAMLINES,
        ARROWS
    };

    VisMode vis_mode = INK;

    float dt = 1.0f / 60.0f;
    float dx = 1.0f;
    const size_t JACOBI_ITERATIONS = 50;

    bool mouse_active = false;
    bool paused = false;

    // Used to prevent large relative movement the first click
    bool click = false;

private:
    void selfAdvectVelocity();
    void diffuseVelocity();
    void applyForce();
    void computeCurl();
    void applyVorticityConfinement();
    void computeDivergence();
    void computePressure();
    void subtractPressureGradient();
    void updateInk();
    void createStreamlines();

    std::shared_ptr<Config> cfg;
    Shader draw_shader;
    Shader advect_shader;
    Shader force_shader;
    Shader jacobi_diffusion_shader;
    Shader jacobi_pressure_shader;
    Shader divergence_shader;
    Shader curl_shader;
    Shader vorticity_shader;
    Shader gradient_subtract_shader;
    Shader add_ink_shader;
    Shader streamlines_shader;
    Quad quad;

    std::unique_ptr<FBO> velocity;
    std::unique_ptr<FBO> divergence;
    std::unique_ptr<FBO> curl;
    std::unique_ptr<FBO> pressure;
    std::unique_ptr<FBO> temp_fbo;

    std::unique_ptr<FBO> noise;
    std::unique_ptr<FBO> ink;
    std::unique_ptr<FBO> streamlines;
    std::unique_ptr<FBO> temp_large;

    void saveRender();
    bool save_next = false;
    std::string savename = "";
};
