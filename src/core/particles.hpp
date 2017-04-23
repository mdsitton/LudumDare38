#pragma once
#include <glm/glm.hpp>

#include <random>
#include <vector>
#include <iostream>

#include "renderer/renderer.hpp"

namespace ORCore
{
    struct Particle
    {
        float mass;
        double lifetime;
        glm::vec2 velocity;
        glm::vec2 position;
    };

    // Emitter interface
    class Emitter
    {
    public:

        // Runs internal particle update logic such as spawning and deleting of particles.
        virtual void update(double dt) = 0;

        // Returns the pointer to the first particle
        virtual std::vector<Particle>& get_particles() = 0;

        virtual void collect_particles() = 0;
    };


    class PointEmitter : public Emitter
    {
    public:
        PointEmitter();

        virtual void update(double dt);
        virtual std::vector<Particle>& get_particles();

        virtual void collect_particles();

        void set_location(int x, int y);
        void set_velocity(glm::vec2 vel);
        void create_particles(double dt);

    private:

        std::mt19937 m_rng;
        std::uniform_real_distribution<float> m_dispersion;

        int m_posX;
        int m_posY;
        int m_creationRate;

        glm::vec2 m_currPos;
        glm::vec2 m_lastPos;

        glm::vec2 m_velocity;
        std::vector<Particle> m_particles;
    };

    class ParticleManager
    {
    public:
        ParticleManager(Renderer* renderer);
        void init_gl();
        void set_program(int program);
        void register_emitter(Emitter* emitter);
        void simulate_particles(double dt);
        void render_update();
    private:
        Renderer *m_renderer;
        int m_program;
        int m_objID;
        std::vector<Emitter*> m_emitters;
    };
}