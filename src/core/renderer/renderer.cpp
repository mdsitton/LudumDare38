#include "config.hpp"
#include "renderer.hpp"
#include <iostream>

namespace ORCore
{

    std::vector<Vertex> create_rect_mesh(glm::vec4 color)
    {
        return {
            // Vertex2     UV            Color
            {{0.0f, 0.0f, 0.5f}, {0.0f, 0.0f}, color},
            {{0.0f, 1.0f, 0.5f}, {0.0f, 1.0f}, color},
            {{1.0f, 0.0f, 0.5f}, {1.0f, 0.0f}, color},
            {{0.0f, 1.0f, 0.5f}, {0.0f, 1.0f}, color},
            {{1.0f, 1.0f, 0.5f}, {1.0f, 1.0f}, color},
            {{1.0f, 0.0f, 0.5f}, {1.0f, 0.0f}, color}
        };
    }

    RenderObject::RenderObject()
    :batchID(-1)
    {

    }

    void RenderObject::set_state(RenderState stateItem, int value)
    {
        try {
            state.at(stateItem) = value;
        } catch (std::out_of_range &err) {
            state.insert({stateItem, value});
        }
    }

    void RenderObject::set_scale(glm::vec3&& scale)
    {
        mesh.scale = scale;
    }

    void RenderObject::set_translation(glm::vec3&& translation)
    {
        mesh.translate = translation;
    }

    void RenderObject::set_primitive_type(Primitive primitive)
    {

        set_state(RenderState::primitive, primitive);

        mesh.primitive = primitive;
        if (primitive == Primitive::triangle)
        {
            mesh.vertexSize = 3;
        } else if (primitive == Primitive::line)
        {
            mesh.vertexSize = 2;

        } else if (primitive == Primitive::point)
        {
            mesh.vertexSize = 1;
        }
    }

    void RenderObject::set_geometry(std::vector<Vertex>&& geometry)
    {
        mesh.vertices = geometry;
    }

    void RenderObject::set_texture(int texture)
    {
        set_state(RenderState::texture, texture);
    }

    void RenderObject::set_program(int program)
    {
        set_state(RenderState::program, program);
    }

    void RenderObject::set_point_size(int pointSize)
    {
        if (mesh.primitive == Primitive::point)
        {
            set_state(RenderState::point_size, pointSize);
        }
    }

    void RenderObject::update()
    {
        modelMatrix = glm::scale(glm::translate(glm::mat4(1.0f), mesh.translate), mesh.scale);
    }


    Renderer::Renderer()
    : m_logger(spdlog::get("default"))
    {

    }

    void Renderer::init_gl()
    {
        // Add the blank texture by default as it will be the default texture.
        m_defaultTextureID = add_texture(ORCore::loadSTB("data/blank.png"));
    }

    int Renderer::create_batch(const std::map<RenderState, int>& batchState, int batchSize)
    {
        try
        {
            int id = m_batches.size();
            m_batches.push_back(
                std::make_unique<Batch>(
                    m_programs[batchState.at(RenderState::program)].get(),
                    m_textures[batchState.at(RenderState::texture)].get(),
                    batchSize, id));

            auto& batch = m_batches.back();
            batch->set_state(batchState);
            return id;
        } catch (std::out_of_range &err) {
            throw std::runtime_error("Error: batch could not be created missing critital data");
        }
    }

    int Renderer::find_batch(const std::map<RenderState, int>& batchState)
    {
        // Find existing batch that isnt full or already submitted.

        for (auto &batch : m_batches)
        {
            auto& bState = batch->get_state();
            if (!batch->is_committed() && bState.size() == batchState.size() &&
                std::equal(std::begin(batchState), std::end(batchState), std::begin(bState),
                    [](auto& a, auto& b){return a.first == b.first && a.second == b.second;}))
            {
                return batch->get_id();
            }
        }


        m_logger->debug("No batches found creating new batch. Total batches: {}", m_batches.size()+1);

        return create_batch(batchState, 2048);
    }

    RenderObject* Renderer::get_object(int objID)
    {
        return &m_objects[objID];
    }


    void Renderer::clear_object_batch(int objID)
    {
        RenderObject& obj = m_objects[objID];
        m_batches[obj.batchID]->clear();
    }

    void Renderer::update_object(int objID)
    {
        RenderObject& obj = m_objects[objID];
        obj.update();
        m_batches[obj.batchID]->update_mesh(obj.mesh, obj.modelMatrix);
    }

    int Renderer::add_object(const RenderObject& objIn)
    {

        int objID = m_objects.size();
        m_objects.push_back(objIn);
        auto &obj = m_objects.back();

        auto &state = obj.state;

        // if there is no texture set it to the default.
        if (state.find(RenderState::texture) == state.end())
        {
            state.insert({RenderState::texture, m_defaultTextureID});
        }

        int batchId = find_batch(state);

        obj.id=objID;
        obj.batchID = batchId;
        obj.update();

        // dont add to batch if we dont have geometry
        // try until it gets added to a batch.
        while (m_batches[batchId]->add_mesh(obj.mesh, obj.modelMatrix) != true)
        {
            m_batches[batchId]->commit(); // commit that batch as it is full.
            batchId = find_batch(objIn.state); // find or create the next batch
            obj.batchID = batchId;
        }

        return obj.id;
    }

    int Renderer::add_object_dedibatch(const RenderObject& objIn)
    {
        int objID = m_objects.size();
        m_objects.push_back(objIn);
        auto &obj = m_objects.back();

        auto &state = obj.state;

        // if there is no texture set it to the default.
        if (state.find(RenderState::texture) == state.end())
        {
            state.insert({RenderState::texture, m_defaultTextureID});
        }

        int batchId;

        if (obj.batchID == -1)
        {
            batchId = create_batch(state, 262144);
        } else {
            batchId = obj.batchID;
        }

        m_batches[batchId]->commit(); // bit of a hack

        obj.id=objID;
        obj.batchID = batchId;
        obj.update();
        return obj.id;
    }

    int Renderer::readd_object(int objID)
    {
        auto &obj = m_objects[objID];
        return m_batches[obj.batchID]->add_mesh(obj.mesh, obj.modelMatrix);
    }

    int Renderer::add_texture(Image&& img)
    {
        int id = m_textures.size();
        m_textures.push_back(std::make_unique<Texture>(GL_TEXTURE_2D));
        auto &texture = m_textures.back();
        texture->update_image_data(img);
        return id;
    }

    int Renderer::add_program(Shader&& vertex, Shader&& fragment)
    {
        int id = m_programs.size();
        m_programs.push_back(std::make_unique<ShaderProgram>(vertex, fragment));
        auto &program = m_programs.back();
        program->check_error();
        return id;
    }

    void Renderer::set_camera_transform(std::string name, glm::mat4&& transform)
    {
        try {
            m_cameraUniforms.at(name) = transform;
        } catch (std::out_of_range &err) {
            m_cameraUniforms.insert({name, transform});
        }
    }

    // commit all remaining batches.
    void Renderer::commit()
    {
        for (auto &batch : m_batches)
        {
            if (!batch->is_committed())
            {
                batch->commit();
            }
        }
        // m_logger->info("Batches: {}", m_batches.size());

        // GLint size;
        // glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &size);
        // m_logger->info("Max buffer texture size: {}", size);
        // glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &size);
        // m_logger->info("Max texture units: {}", size);
        // glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &size);
        // m_logger->info("Max combined texture units: {}", size);
    }

    void Renderer::render()
    {
        // TODO - Do sorting of batches to minimize state changes.
        for (auto &batch : m_batches)
        {
            ShaderProgram* program = batch->get_program();
            program->use();
            for (auto &cam : m_cameraUniforms)
            {
                program->set_uniform(program->uniform_attribute(cam.first), cam.second);
            }
            batch->render();
        }
    }

    void Renderer::clear()
    {
        for (auto &batch : m_batches)
        {
            batch->clear();
        }
    }

    Renderer::~Renderer()
    {

    }


}