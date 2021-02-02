#include "application.hpp"

#include <iomanip>

using namespace nanogui;

Application::PropertySlider::PropertySlider(Widget* window, Config::Property* p, const std::string &name, const std::string &unit, size_t precision)
    : prop(p), last_value(*p), precision(precision)
{
    Widget* panel = new Widget(window);
    panel->set_layout(new GridLayout(Orientation::Horizontal, 3, Alignment::Middle));

    Label* label = new Label(panel, name, "sans-bold");
    label->set_fixed_width(86);

    slider = new Slider(panel);

    slider->set_value(prop->getNormalized());
    slider->set_fixed_size({ 200, 20 });

    text_box = new TextBox(panel);
    text_box->set_fixed_size(Vector2i(70, 20));
    text_box->set_font_size(14);
    text_box->set_alignment(TextBox::Alignment::Right);
    text_box->set_units(unit);

    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << prop->getDisplay();
    text_box->set_value(ss.str());

    slider->set_callback([prop = prop, slider = slider, this](float value)
        {
            // Hack to skip first set event. Could be fixed if nanogui didn't trigger the slider set event on mouse button release.
            if (!initiated)
            {
                initiated = true;
                slider->set_value(prop->getNormalized());
                return;
            }
            prop->setNormalized(value);
        }
    );

    slider->set_final_callback([this](float value) { initiated = false; } );
}


void Application::PropertySlider::updateValue()
{
    if (last_value != *prop)
    {
        float v = prop->getNormalized();
        slider->set_value(v);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision) << prop->getDisplay();
        text_box->set_value(ss.str());
        last_value = *prop;
    }
}