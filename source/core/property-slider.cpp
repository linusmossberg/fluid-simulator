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

    float_box = new FloatBox<float>(panel);
    float_box->set_alignment(TextBox::Alignment::Right);
    float_box->set_fixed_size({ 70, 20 });
    float_box->number_format("%.0" + std::to_string(precision) + "f");
    float_box->set_editable(true);
    float_box->set_value(prop->getDisplay());
    float_box->set_font_size(14);
    float_box->set_units(unit);

    float_box->set_callback([float_box = float_box, prop = prop, precision](float value)
        {
            prop->setDisplay(value);
            float_box->set_value(*prop);
        }
    );
}


void Application::PropertySlider::updateValue()
{
    if (last_value != *prop)
    {
        slider->set_value(prop->getNormalized());
        float_box->set_value(prop->getDisplay());
        last_value = *prop;
    }
}