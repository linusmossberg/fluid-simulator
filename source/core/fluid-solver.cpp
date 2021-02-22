#include "fluid-solver.hpp"

#include <iostream>

#include "util.hpp"
#include "../gl-util/fbo.hpp"
#include "../gl-util/shader.hpp"
#include "../gl-util/quad.hpp"

#include "../shaders/screen.vert"
#include "../shaders/stencil.vert"
#include "../shaders/advect.frag"
#include "../shaders/force.frag"
#include "../shaders/jacobi-diffusion.frag"
#include "../shaders/jacobi-pressure.frag"
#include "../shaders/divergence.frag"
#include "../shaders/curl.frag"
#include "../shaders/vorticity.frag"
#include "../shaders/gradient-subtract.frag"
#include "../shaders/speed.frag"

FluidSolver::FluidSolver(const std::shared_ptr<Config>& cfg) : cfg(cfg)
{ 
    setSize(glm::ivec2(128));
}

void FluidSolver::step()
{
    glViewport(0, 0, grid_cells.x, grid_cells.y);
    glScissor(0, 0, grid_cells.x, grid_cells.y);

    dx = cfg->sim_width / grid_cells.x;

    if (clear_velocity)
    {
        velocity->clear(glm::vec4(0.0f));
        pressure->clear(glm::vec4(0.0f));
        clear_velocity = false;
    }

    Quad::bind();

    applyForce();
    applyVorticityConfinement();
    advect(velocity, temp_fbo);
    diffuseVelocity();
    subtractPressureGradient();

    computeSpeed();
}

void FluidSolver::advect(std::unique_ptr<FBO>& quantity, std::unique_ptr<FBO>& temp_quantity)
{
    static const Shader advect_shader(screen_vert, advect_frag, "advect");

    temp_quantity->bind();

    advect_shader.use();

    glUniform1f(advect_shader.getLocation("inv_dx"), 1.0f / dx);
    glUniform1f(advect_shader.getLocation("dt"), cfg->dt);

    velocity->bindTexture(0, GL_LINEAR);
    quantity->bindTexture(1, GL_LINEAR);

    Quad::draw();

    std::swap(quantity, temp_quantity);
}

void FluidSolver::diffuseVelocity()
{
    static const Shader jacobi_diffusion_shader(stencil_vert, jacobi_diffusion_frag, "jacobi-diffusion");

    float nu = cfg->mu / cfg->rho;

    // Solver approaches unchanged velocity when dx2_nudt 
    // approaches infinity, so this should be correct.
    if (nu <= 0.0f) return;

    jacobi_diffusion_shader.use();

    float dx2_nudt = std::powf(dx, 2) / (nu * cfg->dt);

    glUniform2fv(jacobi_diffusion_shader.getLocation("tx_size"), 1, &cell_size[0]);
    glUniform1f(jacobi_diffusion_shader.getLocation("dx2_nudt"), dx2_nudt);

    velocity->bindTexture(1);

    for (int i = 0; i < cfg->viscosity_iterations; i++)
    {
        temp_fbo->bind();

        velocity->bindTexture(0);

        Quad::draw();

        std::swap(velocity, temp_fbo);
    }
    std::swap(velocity, temp_fbo);
}

void FluidSolver::applyForce()
{
    static const Shader force_shader(screen_vert, force_frag, "force");

    temp_fbo->bind();

    force_shader.use();

    velocity->bindTexture(0);

    glm::vec2 force = (float)cfg->F * glm::vec2(std::cos(cfg->F_angle), std::sin(cfg->F_angle));

    glUniform2fv(force_shader.getLocation("tx_size"), 1, &cell_size[0]);
    glUniform2fv(force_shader.getLocation("pos"), 1, &force_pos[0]);
    glUniform2fv(force_shader.getLocation("force"), 1, &force[0]);
    glUniform1f(force_shader.getLocation("dt"), cfg->dt);

    Quad::draw();

    std::swap(velocity, temp_fbo);
}

void FluidSolver::computeCurl()
{
    static const Shader curl_shader(stencil_vert, curl_frag, "curl");

    curl->bind();

    curl_shader.use();

    velocity->bindTexture(0);

    glUniform2fv(curl_shader.getLocation("tx_size"), 1, &cell_size[0]);
    glUniform1f(curl_shader.getLocation("half_inv_dx"), 0.5f / dx);

    Quad::draw();
}

void FluidSolver::applyVorticityConfinement()
{
    static const Shader vorticity_shader(stencil_vert, vorticity_frag, "vorticity");

    computeCurl();

    if (cfg->vorticity < 1e-6f) return;

    temp_fbo->bind();

    vorticity_shader.use();

    velocity->bindTexture(0);
    curl->bindTexture(1);

    glUniform2fv(vorticity_shader.getLocation("tx_size"), 1, &cell_size[0]);
    glUniform1f(vorticity_shader.getLocation("half_inv_dx"), 0.5f / dx);
    glUniform1f(vorticity_shader.getLocation("dt"), cfg->dt);
    glUniform1f(vorticity_shader.getLocation("vorticity_scale"), cfg->vorticity);

    Quad::draw();

    std::swap(temp_fbo, velocity);
}

void FluidSolver::computeDivergence() 
{
    static const Shader divergence_shader(stencil_vert, divergence_frag, "divergence");

    divergence->bind();

    divergence_shader.use();

    velocity->bindTexture(0);

    glUniform2fv(divergence_shader.getLocation("tx_size"), 1, &cell_size[0]);
    glUniform1f(divergence_shader.getLocation("half_inv_dx"), 0.5f / dx);

    Quad::draw();
}

void FluidSolver::computePressure()
{
    static const Shader jacobi_pressure_shader(stencil_vert, jacobi_pressure_frag, "jacobi-pressure");

    computeDivergence();

    if (clear_pressure) pressure->clear(glm::vec4(0.0f));

    jacobi_pressure_shader.use();

    glUniform2fv(jacobi_pressure_shader.getLocation("tx_size"), 1, &cell_size[0]);
    glUniform1f(jacobi_pressure_shader.getLocation("dx2"), std::powf(dx, 2));

    divergence->bindTexture(1);

    for (int i = 0; i < cfg->pressure_iterations; i++)
    {
        temp_fbo->bind();

        pressure->bindTexture(0);

        Quad::draw();

        std::swap(temp_fbo, pressure);
    }
    std::swap(temp_fbo, pressure);
}

void FluidSolver::subtractPressureGradient() 
{
    static const Shader gradient_subtract_shader(stencil_vert, gradient_subtract_frag, "gradient");

    computePressure();

    temp_fbo->bind();

    gradient_subtract_shader.use();

    pressure->bindTexture(0);
    velocity->bindTexture(1);

    glUniform2fv(gradient_subtract_shader.getLocation("tx_size"), 1, &cell_size[0]);
    glUniform1f(gradient_subtract_shader.getLocation("half_inv_dx"), 0.5f / dx);

    Quad::draw();

    std::swap(temp_fbo, velocity);
}

void FluidSolver::computeSpeed()
{
    static const Shader speed_shader(screen_vert, speed_frag, "speed");

    speed->bind();
    speed_shader.use();
    velocity->bindTexture(0);
    Quad::draw();
}

void FluidSolver::setSize(const glm::ivec2& grid_cells_)
{
    grid_cells = grid_cells_;

    cell_size = 1.0f / glm::vec2(grid_cells);

    for (auto& fbo : { &temp_fbo, &velocity, &pressure, &divergence, &curl, &speed })
    {
        *fbo = std::make_unique<FBO>(grid_cells);
    }
}