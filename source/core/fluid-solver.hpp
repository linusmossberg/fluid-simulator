#pragma once

#include <memory>

#include "config.hpp"

#include "../gl-util/quad.hpp"

class Config;
class FBO;

class FluidSolver
{
public:
    FluidSolver(const std::shared_ptr<Config>& cfg);

    void setSize(const glm::ivec2& grid_cells);

    void step();

    void advect(std::unique_ptr<FBO>& quantity, std::unique_ptr<FBO>& temp_quantity);
    void diffuseVelocity();
    void applyForce();
    void applyVorticityConfinement();
    void subtractPressureGradient();

    void computeCurl();
    void computeDivergence();
    void computePressure();
    void computeSpeed();

    void clearFBO(std::unique_ptr<FBO>& fbo, const glm::vec4& clear_color);

    std::shared_ptr<Config> cfg;

    bool clear_pressure = false;
    glm::ivec2 grid_cells;
    glm::vec2 force_pos = glm::vec2(0.5, 0.5);
    float dx = 1.0f;
    const size_t JACOBI_ITERATIONS = 50;

    glm::vec2 cell_size;

    Quad quad;

    std::unique_ptr<FBO> velocity;
    std::unique_ptr<FBO> divergence;
    std::unique_ptr<FBO> curl;
    std::unique_ptr<FBO> pressure;
    std::unique_ptr<FBO> speed;
    std::unique_ptr<FBO> temp_fbo;
};
