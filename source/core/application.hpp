#include <nanogui/nanogui.h>

#include "config.hpp"

class FluidSimulator;

class Application : public nanogui::Screen 
{
public:
    Application();

    virtual void draw(NVGcontext *ctx);

private:
    FluidSimulator *fluid_simulator;
    std::shared_ptr<Config> cfg;

    struct PropertySlider
    {
        PropertySlider(nanogui::Widget* window, Config::Property* p, const std::string &name, 
                       const std::string &unit, size_t precision);

        nanogui::Slider* slider;
        //nanogui::TextBox* text_box;
        nanogui::FloatBox<float>* float_box;
        Config::Property* const prop;

        float last_value;
        size_t precision;
        bool initiated = false;

        void updateValue();
    };

    struct PropertyBoxRow
    {
        PropertyBoxRow(nanogui::Widget* window, const std::vector<Config::Property*> &properties, 
                       const std::string &name, const std::string &unit, size_t precision, float step,
                       std::string tooltip = "", size_t total_width = 270);

        std::vector<Config::Property*> properties;
        std::vector<nanogui::FloatBox<float>*> float_boxes;
        std::vector<float> last_values;

        void updateValues();
    };

    std::vector<PropertySlider> sliders;
    std::vector<PropertyBoxRow> float_box_rows;
};