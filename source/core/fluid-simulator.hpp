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
    glm::ivec2 mouse_pos;

    double last_time = std::numeric_limits<double>::max();

    bool mouse_active = false;

    bool visualize_autofocus = false;

    // Used to prevent large relative movement the first click
    bool click = false;

private:
    std::shared_ptr<Config> cfg;
    Shader draw_shader;
    Quad quad;
    std::unique_ptr<FBO> fbo0;
    std::unique_ptr<FBO> fbo1;

    void saveRender();
    bool save_next = false;
    std::string savename = "";
};
