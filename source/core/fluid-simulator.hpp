#pragma once

#include <nanogui/canvas.h>

#include <glm/glm.hpp>

#include "../gl-util/quad.hpp"
#include "../gl-util/texture-1d.hpp"

#include "fluid-solver.hpp"

class FBO;
class Config;

class FluidSimulator : public nanogui::Canvas
{
public:
    FluidSimulator(Widget* parent, const std::shared_ptr<Config>& cfg);

    virtual void draw_contents() override;

    void setColorMap(int index);

    void resize();
    void saveNextRender(const std::string& filename);

    virtual bool mouse_drag_event(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers) override;
    virtual bool mouse_button_event(const nanogui::Vector2i &p, int button, bool down, int modifiers) override;

    FluidSolver fluid_solver;

    glm::ivec2 fb_size;
    glm::vec2 mouse_pos = glm::vec2(0.05,0.5);

    bool auto_set_range = true;

    double last_time = std::numeric_limits<double>::max();

    float sim_time = 0.0f;

    enum VisMode
    {
        INK,
        STREAMLINES,
        ARROWS,
        CURL,
        PRESSURE,
        SPEED
    };

    VisMode vis_mode = INK;

    bool fixed_dt = true;

    bool mouse_active = false;
    bool paused = false;

    // Used to prevent large relative movement the first click
    bool click = false;

private:
    void updateInk();
    void createStreamlines();
    void drawColorMap(const std::unique_ptr<FBO>& scalar_field);
    void setColorMapRange();

    std::shared_ptr<Config> cfg;

    Quad quad;

    std::unique_ptr<FBO> noise;
    std::unique_ptr<FBO> ink;
    std::unique_ptr<FBO> streamlines;
    std::unique_ptr<FBO> temp_fbo;

    Texture1D transfer_function;

    void saveRender();
    bool save_next = false;
    std::string savename = "";
};
