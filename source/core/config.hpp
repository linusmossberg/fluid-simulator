#pragma once

class Config
{
public:
    Config() : 
        nu(0.5f, 0.001f, 1000.0f),
        width(512.0f, 400.0f, 1000.0f),
        height(512.0f, 400.0f, 1000.0f)
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

    Property nu;
    Property width;
    Property height;
};