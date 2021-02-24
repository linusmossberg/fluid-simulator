#pragma once

#include <glm/glm.hpp>

class Config
{
public:
    Config() : 
        mu(1.0f, 0.0f, 1e4f, 1e-3f), // Display as centipoise
        rho(1.0f, 0.1f, 10.0f, 1e3f), // Display as kg/L
        vorticity(0.05f, 0.0f, 0.2f),
        width(820.0f, 400.0f, 1000.0f),
        height(640.0f, 400.0f, 1000.0f),
        sim_downscale(4.0f, 1.0f, 8.0f),
        F(0.03f, 0.0f, 0.5f),
        F_angle(0.0f, 0.0f, 360.0f, glm::radians(1.0f)),
        sim_width(2.0f, 1.0f, 3.0f),
        range_min(0.0f, -1e3f, 1e3f),
        range_max(1.0f, -1e3f, 1e3f),
        dt(1.0f / 60.0f, 1e-5f, 1e-1f),
        pressure_iterations(50.0f, 10.0f, 500.0f),
        viscosity_iterations(50.0f, 10.0f, 500.0f),
        ink_rate(1.0f, 0.0f, 2.0f),
        ink_exposure(0.0f, -10.0f, 10.0f),
        arrow_scale(1.0f, 0.1f, 2.0f),
        arrow_cols(16.0f, 8.0f, 128.0f)
    { }

    struct Property
    {
        Property(float value, float min, float max, float scale = 1.0f)
            : value(value * scale), min(min * scale), max(max * scale), range((max - min) * scale), scale(scale) { }

        operator float() const { return value; }

        void operator=(const float &v)
        {
            if (v > max) value = max;
            else if (v < min) value = min;
            else value = v;
        }

        void operator+=(const float &v) { *this = value + v; }
        void operator-=(const float &v) { *this = value - v; }

        float getRange() { return range; }
        float getNormalized() { return (value - min) / range; }
        float getDisplay() { return value / scale; }
        float getScale() { return scale; }

        bool valid() { return min <= value && max >= value; }

        void setNormalized(float v) { *this = min + v * range; }
        void setDisplay(float v) { *this = v * scale; }

    private:
        float value, min, max, range, scale;
    };

    Property mu;
    Property rho;
    Property vorticity;
    Property width;
    Property height;
    Property sim_downscale;
    Property F;
    Property F_angle;
    Property sim_width;
    Property range_min;
    Property range_max;
    Property dt;
    Property pressure_iterations;
    Property viscosity_iterations;
    Property ink_rate;
    Property ink_exposure;
    Property arrow_scale;
    Property arrow_cols;
};