#include "application.hpp"

#include <iostream>

#include <nanogui/opengl.h>

#include "fluid-simulator.hpp"

Application::Application() : 
    Screen(nanogui::Vector2i(1280, 720), "fluid-simulator", true, false, false, false, false, 4U, 3U), 
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

    addText("Light Field Renderer", "sans-bold", 24);
    addText("github.com/linusmossberg/light-field-renderer", "sans", 16);
    addText(" ");

    addControl("W", "Move forward");
    addControl("A", "Move left");
    addControl("S", "Move back");
    addControl("D", "Move right");
    addControl("SPACE", "Move up");
    addControl("CTRL", "Move down");
    addControl("SCROLL", "Change focus distance");
    addControl("SHIFT + CLICK", "Autofocus");
    addControl("MOUSE DRAG", "Mouse mode");

    panel = new nanogui::Widget(window);
    panel->set_layout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Fill, 0, 5));

    label = new nanogui::Label(panel, "Light Field", "sans-bold");
    label->set_fixed_width(86);

    panel = new nanogui::Widget(window);
    panel->set_layout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 4, nanogui::Alignment::Fill));
    label = new nanogui::Label(panel, "Render Size", "sans-bold");
    label->set_fixed_width(86);

    float_box_rows.push_back(PropertyBoxRow(panel, { &cfg->width, &cfg->height }, "", "px", 0, 1.0f, "", 180));

    b = new nanogui::Button(panel, "Set");
    b->set_font_size(16);
    b->set_fixed_size({ 90, 20 });
    b->set_callback([this, window]
        { 
            fluid_simulator->resize();
            perform_layout();
        }
    );

    sliders.emplace_back(window, &cfg->nu, "nu", "a", 4);

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
