#include <algorithm>
#include <iostream>
#include "particles.hpp"

namespace ORCore
{
    // PointEmitter

    PointEmitter::PointEmitter()
    :m_rng(std::random_device()()),
    m_dispersion(-0.1f, 0.1f),
    m_posX(0),
    m_posY(0),
    m_creationRate(1)
    {
    }

    void PointEmitter::update(double dt)
    {
        create_particles(dt);
    }

    std::vector<Particle>& PointEmitter::get_particles()
    {
        return m_particles;
    }

    void PointEmitter::set_location(int x, int y)
    {
        m_lastPos = m_currPos;
        m_currPos = glm::vec2{x, y};
    }

    void PointEmitter::set_velocity(glm::vec2 vel)
    {
        m_velocity = vel;
    }

    void PointEmitter::create_particles(double dt)
    {
        int count = m_creationRate*dt;
        for (int i = 0; i < count; i++)
        {
            m_particles.push_back({1.0f, 5.0, glm::vec2{m_dispersion(m_rng), m_dispersion(m_rng)} + m_velocity, m_currPos});
        }
    }

    void PointEmitter::collect_particles()
    {

        // auto test = [](auto& x)
        // {
        //     return x.lifetime <= 0.0;
        // };

        // auto remove = std::remove_if(m_particles.begin(), m_particles.end(), test);


        // m_particles.erase(remove, m_particles.end());
    }


    // Particle Manager

    ParticleManager::ParticleManager(Renderer* renderer)
    :m_renderer(renderer)
    {
    }

    void ParticleManager::set_program(int program)
    {
        m_program = program;
    }

    void ParticleManager::init_gl()
    {
        ORCore::RenderObject obj;
        obj.set_program(m_program);
        obj.set_scale(glm::vec3{1.0f, 1.0f, 0.0f});
        obj.set_translation(glm::vec3{0.0f, 0.0f, 0.0f});
        obj.set_primitive_type(ORCore::Primitive::point);
        obj.set_point_size(4);

        m_objID = m_renderer->add_object_dedibatch(obj);
    }


    void ParticleManager::register_emitter(Emitter* emitter)
    {
        m_emitters.push_back(emitter);
    }


    void ParticleManager::simulate_particles(double dt)
    {
        for (auto *emitter : m_emitters)
        {
            emitter->update(dt);

            for(auto &particle : emitter->get_particles())
            {
                particle.lifetime -= dt;
                particle.velocity += (glm::vec2(0.0, particle.mass * 9.81f) / particle.mass) * (float)dt;
                particle.position += particle.velocity * (float)dt; // TODO - fix c-style casts < ^
            }

            emitter->collect_particles();
        }
    }

    void ParticleManager::render_update()
    {
        std::vector<Vertex> points;

        for (auto *emitter : m_emitters)
        {
            points.clear();

            std::vector<Particle> &particles = emitter->get_particles();
            points.reserve(particles.size());

            for(auto &particle : particles)
            {
                points.push_back(Vertex{{particle.position.x, particle.position.y, 0.5f}, {0.0f, 0.0f}, glm::vec4{1.0f,0.0f,0.0f,0.0f}});
            }
        }
        RenderObject *robj = m_renderer->get_object(m_objID);
        m_renderer->clear_object_batch(robj->id);
        robj->set_geometry(std::move(points));

        m_renderer->readd_object(robj->id);
    }
}