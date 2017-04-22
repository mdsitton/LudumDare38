#include "config.hpp"
#include "game.hpp"

#include <iostream>
#include <stdexcept>

#include "vfs.hpp"
namespace ORGame
{
    GameManager::GameManager()
    :m_width(800),
    m_height(600),
    m_fullscreen(false),
    m_title("Ludum Dare"),
    m_window(m_width, m_height, m_fullscreen, m_title),
    m_context(3, 2, 0),
    m_eventManager(),
    m_eventPump(&m_eventManager)
    {
        m_running = true;

        m_logger = spdlog::get("default");


        m_window.make_current(&m_context);

        m_window.disable_sync(); // only works on osx currently

        // todo fix this.
        //VFS.AddLoader(new ttvfs::DiskLoader);

        //
        // AppPath gets mounted on osx
        // BasePath gets mounted and overwrites any files similar to those in AppPath (data)
        // HomePath gets mounted and overwrites any files similar to those in BasePath (configs)
        // std::cout << ORCore::GetBasePath() << std::endl;
    // #if OSX_APP_BUNDLE
    //     ORCore::mount(ORCore::GetAppPath(), "");
    // #endif
    //     ORCore::mount(ORCore::GetBasePath(), "/bob");
    //     std::vector<std::string> paths = ORCore::resolveSystemPath("/bob");
    //     for (auto &i: paths) {
    //         auto bob = ORCore::sysGetPathContents(i);
    //         std::cout << i << " " << bob.size() << std::endl;
    //     }
        //VFS.Mount( ORCore::GetBasePath().c_str(), "" );
        //VFS.Mount( ORCore::GetHomePath().c_str(), "" );

        //ORCore::mount( "./data", "data" );


        if(!gladLoadGL())
        {
            throw std::runtime_error("Error: GLAD failed to load.");
        }

        m_lis.handler = std::bind(&GameManager::event_handler, this, std::placeholders::_1);
        m_lis.mask = ORCore::EventType::EventAll;


        m_eventManager.add_listener(m_lis);

        m_fpsTime = 0.0;

        m_ss = std::cout.precision();

        ORCore::ShaderInfo vertInfo {GL_VERTEX_SHADER, "./data/shaders/main.vs"};
        ORCore::ShaderInfo fragInfo {GL_FRAGMENT_SHADER, "./data/shaders/main.fs"};

        m_program = m_renderer.add_program(ORCore::Shader(vertInfo), ORCore::Shader(fragInfo));
        m_texture = m_renderer.add_texture(ORCore::loadSTB("data/blank.png"));
        m_texture2 = m_renderer.add_texture(ORCore::loadSTB("data/icon.png"));

        resize(m_width, m_height);

        prep_render_obj();
        m_renderer.commit();

        glClearColor(0.5, 0.5, 0.5, 1.0);
    }

    GameManager::~GameManager()
    {
        m_window.make_current(nullptr);
    }

    void GameManager::prep_render_obj()
    {

        // reuse the same container when creating multiple as add_obj wont modify the original.
        ORCore::RenderObject obj;
        obj.set_texture(m_texture2);
        obj.set_program(m_program);

        obj.set_scale(glm::vec3{512.0f, 100.0f, 0.0f});
        obj.set_translation(glm::vec3{(m_width/2.0f)-256, 100.0f, 0.0f}); // center the line on the screen
        obj.set_primitive_type(ORCore::Primitive::triangle);
        obj.set_geometry(ORCore::create_rect_mesh(glm::vec4{1.0,1.0,1.0,1.0}));

        m_renderer.add_object(obj);
    }

    void GameManager::start()
    {
        GLenum error;
        while (m_running)
        {
            m_fpsTime += m_clock.tick();
            m_eventPump.process();

            update();
            render();

            do
            {
                error = glGetError();
                if (error != GL_NO_ERROR)
                {
                    std::cout << error << std::endl;
                }
            } while(error != GL_NO_ERROR);

            m_window.flip();
            if (m_fpsTime >= 2000.0) {
                std::cout.precision (5);
                std::cout << "FPS: " << m_clock.get_fps() << std::endl;
                std::cout.precision (m_ss);
                m_fpsTime = 0;
            }
        }
    }

    void GameManager::resize(int width, int height)
    {
        m_width = width;
        m_height = height;
        glViewport(0, 0, m_width, m_height);
        m_ortho = glm::ortho(0.0f, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, -1.0f, 1.0f);
    }

    bool GameManager::event_handler(const ORCore::Event &event)
    {
        switch(event.type) {
            case ORCore::Quit: {
                m_running = false;
                break;
            }
            case ORCore::MouseMove: {
                auto ev = ORCore::event_cast<ORCore::MouseMoveEvent>(event);
                //std::cout << "mouse x: " << ev.x << " mouse y" << ev.y << std::endl;
                m_mouseX = ev.x;
                m_mouseY = ev.y;
                break;
            }
            case ORCore::WindowSize: {
                auto ev = ORCore::event_cast<ORCore::WindowSizeEvent>(event);
                resize(ev.width, ev.height);
                break;
            }
            case ORCore::KeyDown: {
                auto ev = ORCore::event_cast<ORCore::KeyDownEvent>(event);
                switch(ev.key) {
                    case ORCore::KeyCode::KEY_F:
                        std::cout << "Key F" << std::endl;
                        break;
                    default:
                        std::cout << "Other Key" << std::endl;
                        break;
                }
            }
            default:
                break;
        }
        return true;
    }

    void GameManager::update()
    {

        m_renderer.set_camera_transform("ortho", glm::translate(m_ortho, glm::vec3(0.0f, 0.0f, 0.0f))); // translate projection with song
    }

    void GameManager::render()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_renderer.render();
    }
}