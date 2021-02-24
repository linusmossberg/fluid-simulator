#include "application.hpp"

#include <iostream>

#include <nanogui/opengl.h>

#include "fluid-simulator.hpp"
#include "color-maps.hpp"

Application::Application() : 
    Screen(nanogui::Vector2i(1280, 720), "fluid-simulator", true, false, true, false, false, 4U, 3U), 
    cfg(std::make_shared<Config>())
{
    inc_ref();

    auto theme = new nanogui::Theme(this->nvg_context());
    theme->m_window_fill_focused = nanogui::Color(45, 255);
    theme->m_window_fill_unfocused = nanogui::Color(45, 255);
    theme->m_drop_shadow = nanogui::Color(0, 0);

    nanogui::Window *window;
    nanogui::Button* b;
    nanogui::Widget* panel;
    nanogui::Label* label;

    window = new nanogui::Window(this, "Render");
    window->set_position(nanogui::Vector2i(410, 10));
    window->set_layout(new nanogui::GroupLayout());
    window->set_theme(theme);

    fluid_simulator = new FluidSimulator(window, cfg);
    fluid_simulator->set_visible(true);

    b = new nanogui::Button(window->button_panel(), "", FA_CAMERA);
    b->set_tooltip("Save Screenshot");
    b->set_callback([this]
    {
        std::string path = nanogui::file_dialog({{"tga", ""}}, true);
        if (path.empty()) return;
        fluid_simulator->saveNextRender(path);
    });

    window = new nanogui::Window(this, "Menu");
    window->set_position({ 10, 10 });
    window->set_layout(new nanogui::GroupLayout(15, 6, 15, 0));
    window->set_theme(theme);

    nanogui::PopupButton *info = new nanogui::PopupButton(window->button_panel(), "", FA_QUESTION);
    info->set_font_size(15);
    info->set_tooltip("Info");
    nanogui::Popup *info_panel = info->popup();

    info_panel->set_layout(new nanogui::GroupLayout());

    auto addText = [&info_panel](std::string text, std::string font = "sans", int font_size = 18) 
    {
        auto row = new Widget(info_panel);
        row->set_layout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Middle, 0, 10));
        new nanogui::Label(row, text, font, font_size);
    };

    auto addControl = [&info_panel](std::string keys, std::string desc) 
    {
        auto row = new nanogui::Widget(info_panel);
        row->set_layout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Fill, 0, 10));
        auto desc_widget = new nanogui::Label(row, keys, "sans-bold");
        desc_widget->set_fixed_width(140);
        new nanogui::Label(row, desc);
        return desc_widget;
    };

    addText("Fluid Simulator", "sans-bold", 24);
    addText("github.com/linusmossberg/fluid-simulator", "sans", 16);
    addText(" ");

    addControl("MOUSE DRAG", "Move source");

    panel = new nanogui::Widget(window);
    panel->set_layout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Fill, 0, 5));

    panel = new nanogui::Widget(window);
    panel->set_layout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 5, nanogui::Alignment::Fill));
    label = new nanogui::Label(panel, "Render Size", "sans-bold");
    label->set_fixed_width(86);

    float_box_rows.push_back(PropertyBoxRow(panel, { &cfg->width, &cfg->height }, "", "px", 0, 1.0f, "", 130));
    float_box_rows.push_back(PropertyBoxRow(panel, { &cfg->sim_downscale }, "", "sim 1/x", 0, 1.0f, "", 75));

    b = new nanogui::Button(panel, "Set");
    b->set_font_size(16);
    b->set_fixed_size({ 65, 20 });
    b->set_callback([this, window]
        { 
            fluid_simulator->resize();
            perform_layout();
        }
    );

    panel = new nanogui::Widget(window);
    panel->set_layout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 4, nanogui::Alignment::Fill));
    label = new nanogui::Label(panel, "Time Step", "sans-bold");
    label->set_fixed_width(86);

    float_box_rows.push_back(PropertyBoxRow(panel, { &cfg->dt }, "", "s", 6, 0.001f, "", 90));

    b = new nanogui::Button(panel, "Realtime");
    b->set_flags(nanogui::Button::Flags::ToggleButton);
    b->set_pushed(!fluid_simulator->fixed_dt);
    b->set_fixed_size({ 90, 20 });
    b->set_font_size(16);
    b->set_change_callback([this](bool state) { fluid_simulator->fixed_dt = !state; });

    b = new nanogui::Button(panel, "Paused");
    b->set_flags(nanogui::Button::Flags::ToggleButton);
    b->set_pushed(fluid_simulator->paused);
    b->set_fixed_size({ 90, 20 });
    b->set_font_size(16);
    b->set_change_callback([this](bool state) { fluid_simulator->paused = state; });

    sliders.emplace_back(window, &cfg->mu, "Viscosity", "cP", 2);
    sliders.emplace_back(window, &cfg->rho, "Density", "kg/L", 2);
    sliders.emplace_back(window, &cfg->vorticity, "Vorticity", "N", 2);
    sliders.emplace_back(window, &cfg->sim_width, "Domain Width", "m", 1);
    sliders.emplace_back(window, &cfg->F, "Force", "N", 2);
    sliders.emplace_back(window, &cfg->F_angle, "Force Angle", "", 0);
    sliders.emplace_back(window, &cfg->pressure_iterations, "Pressure Steps", "", 0);
    sliders.emplace_back(window, &cfg->viscosity_iterations, "Visc. Steps", "", 0);

    panel = new nanogui::Widget(window);
    panel->set_layout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 4, nanogui::Alignment::Fill));
    label = new nanogui::Label(panel, "Clear", "sans-bold");
    label->set_fixed_width(86);

    b = new nanogui::Button(panel, "Pressure*");
    b->set_flags(nanogui::Button::Flags::ToggleButton);
    b->set_pushed(fluid_simulator->fluid_solver.clear_pressure);
    b->set_fixed_size({ 83, 20 });
    b->set_font_size(16);
    b->set_change_callback([this](bool state) { fluid_simulator->fluid_solver.clear_pressure = state; });
    b->set_tooltip("Clears the pressure each iteration when enabled. This reduces oscillations but it requires more pressure iterations.");

    b = new nanogui::Button(panel, "Ink");
    b->set_fixed_size({ 83, 20 });
    b->set_font_size(16);
    b->set_callback([this]() { fluid_simulator->clear_ink = true; });

    b = new nanogui::Button(panel, "Velocity");
    b->set_fixed_size({ 83, 20 });
    b->set_font_size(16);
    b->set_callback([this]() { fluid_simulator->fluid_solver.clear_velocity = true; });

    new nanogui::Label(window, "Visualization", "sans-bold", 20);

    panel = new nanogui::Widget(window);
    panel->set_layout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 4, nanogui::Alignment::Fill, 0, 5));

    label = new nanogui::Label(panel, "Mode", "sans-bold");
    label->set_fixed_width(86);

    nanogui::Button* ink = new nanogui::Button(panel, "Ink");
    ink->set_flags(nanogui::Button::Flags::ToggleButton);
    ink->set_pushed(fluid_simulator->vis_mode == FluidSimulator::VisMode::INK);
    ink->set_fixed_size({ 83, 20 });
    ink->set_font_size(16);

    nanogui::Button* streamlines = new nanogui::Button(panel, "Streamlines");
    streamlines->set_flags(nanogui::Button::Flags::ToggleButton);
    streamlines->set_pushed(fluid_simulator->vis_mode == FluidSimulator::VisMode::STREAMLINES);
    streamlines->set_fixed_size({ 83, 20 });
    streamlines->set_font_size(16);

    nanogui::Button* arrows = new nanogui::Button(panel, "Arrows");
    arrows->set_flags(nanogui::Button::Flags::ToggleButton);
    arrows->set_pushed(fluid_simulator->arrow_overlay);
    arrows->set_fixed_size({ 83, 20 });
    arrows->set_font_size(16);
    arrows->set_change_callback([this](bool state) { fluid_simulator->arrow_overlay = state; });

    panel = new nanogui::Widget(window);
    panel->set_layout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 4, nanogui::Alignment::Fill, 0, 5));

    label = new nanogui::Label(panel, "", "sans-bold");
    label->set_fixed_width(86);

    nanogui::Button* curl = new nanogui::Button(panel, "Curl");
    curl->set_flags(nanogui::Button::Flags::ToggleButton);
    curl->set_pushed(fluid_simulator->vis_mode == FluidSimulator::VisMode::CURL);
    curl->set_fixed_size({ 83, 20 });
    curl->set_font_size(16);

    nanogui::Button* pressure = new nanogui::Button(panel, "Pressure");
    pressure->set_flags(nanogui::Button::Flags::ToggleButton);
    pressure->set_pushed(fluid_simulator->vis_mode == FluidSimulator::VisMode::PRESSURE);
    pressure->set_fixed_size({ 83, 20 });
    pressure->set_font_size(16);

    nanogui::Button* speed = new nanogui::Button(panel, "Speed");
    speed->set_flags(nanogui::Button::Flags::ToggleButton);
    speed->set_pushed(fluid_simulator->vis_mode == FluidSimulator::VisMode::SPEED);
    speed->set_fixed_size({ 83, 20 });
    speed->set_font_size(16);

    ink->set_change_callback
    (
        [this, ink, streamlines, curl, pressure, speed](bool state)
        {
            fluid_simulator->vis_mode = FluidSimulator::VisMode::INK;
            ink->set_pushed(true);
            streamlines->set_pushed(false);
            curl->set_pushed(false);
            pressure->set_pushed(false);
            speed->set_pushed(false);
        }
    );
    streamlines->set_change_callback
    (
        [this, ink, streamlines, curl, pressure, speed](bool state)
        {
            fluid_simulator->vis_mode = FluidSimulator::VisMode::STREAMLINES;
            ink->set_pushed(false);
            streamlines->set_pushed(true);
            curl->set_pushed(false);
            pressure->set_pushed(false);
            speed->set_pushed(false);
        }
    );
    curl->set_change_callback
    (
        [this, ink, streamlines, curl, pressure, speed](bool state)
        {
            fluid_simulator->vis_mode = FluidSimulator::VisMode::CURL;
            ink->set_pushed(false);
            streamlines->set_pushed(false);
            curl->set_pushed(true);
            pressure->set_pushed(false);
            speed->set_pushed(false);
        }
    );
    pressure->set_change_callback
    (
        [this, ink, streamlines, curl, pressure, speed](bool state)
        {
            fluid_simulator->vis_mode = FluidSimulator::VisMode::PRESSURE;
            ink->set_pushed(false);
            streamlines->set_pushed(false);
            curl->set_pushed(false);
            pressure->set_pushed(true);
            speed->set_pushed(false);
        }
    );
    speed->set_change_callback
    (
        [this, ink, streamlines, curl, pressure, speed](bool state)
        {
            fluid_simulator->vis_mode = FluidSimulator::VisMode::SPEED;
            ink->set_pushed(false);
            streamlines->set_pushed(false);
            curl->set_pushed(false);
            pressure->set_pushed(false);
            speed->set_pushed(true);
        }
    );

    panel = new nanogui::Widget(window);
    panel->set_layout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Fill, 0, 5));

    label = new nanogui::Label(panel, "Color Map", "sans-bold");
    label->set_fixed_width(86);

    auto* color_maps = new nanogui::ComboBox(panel, ColorMap::color_map_strings);
    color_maps->set_fixed_size({ 260, 20 });
    color_maps->set_callback([this](int index) { fluid_simulator->setColorMap(index); });
    color_maps->set_font_size(16);
    for (const auto& c : color_maps->popup()->children())
    {
        c->set_font_size(color_maps->font_size());
    }

    panel = new nanogui::Widget(window);
    panel->set_layout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 4, nanogui::Alignment::Fill));
    label = new nanogui::Label(panel, "Range", "sans-bold");
    label->set_fixed_width(86);

    float_box_rows.push_back(PropertyBoxRow(panel, { &cfg->range_min, &cfg->range_max }, "", "", 7, 0.01f, "", 180));

    b = new nanogui::Button(panel, "Auto Set");
    b->set_flags(nanogui::Button::Flags::ToggleButton);
    b->set_pushed(fluid_simulator->auto_set_range);
    b->set_fixed_size({ 73, 20 });
    b->set_font_size(16);
    b->set_change_callback([this](bool state) { fluid_simulator->auto_set_range = state; });

    panel = new nanogui::Widget(window);
    panel->set_layout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 3, nanogui::Alignment::Fill));
    label = new nanogui::Label(panel, "Ink Options", "sans-bold");
    label->set_fixed_width(86);

    b = new nanogui::Button(panel, "Filmic Tonemap");
    b->set_flags(nanogui::Button::Flags::ToggleButton);
    b->set_pushed(fluid_simulator->tonemap);
    b->set_fixed_size({ 130, 20 });
    b->set_font_size(16);
    b->set_change_callback([this](bool state) { fluid_simulator->tonemap = state; });

    b = new nanogui::Button(panel, "Load Image", FA_FOLDER_OPEN);
    b->set_fixed_size({ 130, 20 });
    b->set_font_size(16);
    b->set_callback([this]
        {
            std::string path = nanogui::file_dialog
            (
                {
                    {"jpeg", ""}, { "jpg","" }, {"png",""},
                    {"tga",""}, {"bmp",""}, {"psd",""}, {"gif",""},
                    {"hdr",""}, {"pic",""}, {"pnm", ""}
                }, false
            );

            if (path.empty()) return;

            fluid_simulator->ink_image_path = path;
        }
    );

    panel = new nanogui::Widget(window);
    panel->set_layout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Fill));
    label = new nanogui::Label(panel, "Arrow Color", "sans-bold");
    label->set_fixed_width(86);

    auto cp = new nanogui::ColorPicker(panel, { 0, 0, 0, 255 });
    cp->set_fixed_size({ 260, 20 });
    cp->set_final_callback([this](const nanogui::Color& c)
        {
            fluid_simulator->arrow_color = glm::vec3(c.r(), c.g(), c.b());
        }
    );
    auto c = cp->color();
    fluid_simulator->arrow_color = glm::vec3(c.r(), c.g(), c.b());

    sliders.emplace_back(window, &cfg->arrow_cols, "Num Arrows", "", 0);
    sliders.emplace_back(window, &cfg->arrow_scale, "Arrow Scale", "", 1);
    sliders.emplace_back(window, &cfg->ink_rate, "Ink Rate", "", 2);
    sliders.emplace_back(window, &cfg->ink_exposure, "Ink Exposure", "EV", 1);

    perform_layout();
}

void Application::draw(NVGcontext *ctx)
{
    for (auto &s : sliders)
    {
        s.updateValue();
    }

    for (auto &t : float_box_rows)
    {
        t.updateValues();
    }

    Screen::draw(ctx);
}
