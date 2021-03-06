#pragma once
#include <vector>
#include <ios>

#include "window.hpp"
#include "context.hpp"
#include "events.hpp"
#include "timing.hpp"
#include "renderer/shader.hpp"
#include "renderer/renderer.hpp"
#include "renderer/texture.hpp"
#include "particles.hpp"

#include <spdlog/spdlog.h>

namespace PlanetGame
{
    class GameManager
    {
    public:
        GameManager();
        ~GameManager();
        void start();
        bool event_handler(const ORCore::Event &event);
        void handle_song();
        void update(double dt);
        void prep_render_obj();
        void render();
        void resize(int width, int height);
    private:
        bool m_running;
        double m_fpsTime;
        int m_width;
        int m_height;
        bool m_fullscreen;
        std::string m_title;

        int m_mouseX = 0;
        int m_mouseY = 0;

        ORCore::FpsTimer m_clock;

        ORCore::Window m_window;
        ORCore::Context m_context;
        ORCore::EventManager m_eventManager;
        ORCore::EventPumpSDL2 m_eventPump;
        ORCore::Renderer m_renderer;
        ORCore::Listener m_lis;
        ORCore::ParticleManager m_particles;
        ORCore::PointEmitter m_emitter;

        int m_texture;
        int m_texture2;
        int m_program;

        int m_boxID;

        std::shared_ptr<spdlog::logger> m_logger;

        std::streamsize m_ss;
        glm::mat4 m_ortho;
    };
}