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

        int id = m_batches.size();

        m_logger->debug("No batches found creating new batch. Total batches: {}", m_batches.size()+1);

        try
        {
            m_batches.push_back(
                std::make_unique<Batch>(
                    m_programs[batchState.at(RenderState::program)].get(),
                    m_textures[batchState.at(RenderState::texture)].get(),
                    2048, id));

            auto& batch = m_batches.back();
            batch->set_state(batchState);
            return id;
        } catch (std::out_of_range &err) {
            throw std::runtime_error("Error: batch could not be created missing critital data");
        }
    }

    RenderObject* Renderer::get_object(int objID)
    {
        return &m_objects[objID];
    }

    void Renderer::update_object(int objID)
    {
        RenderObject& obj = m_objects[objID];
        obj.update();
        m_batches[obj.batchID]->update_mesh(obj.mesh, obj.modelMatrix);
    }

    int Renderer::add_object(const RenderObject& objIn)
    {

        int batchId = find_batch(objIn.state);

        int objID = m_objects.size();
        m_objects.push_back(objIn);
        auto &obj = m_objects.back();

        obj.id=objID;
        obj.batchID = batchId;
        obj.update();

        // try until it gets added to a batch.
        while (m_batches[batchId]->add_mesh(obj.mesh, obj.modelMatrix) != true)
        {
            m_batches[batchId]->commit(); // commit that batch as it is full.
            batchId = find_batch(objIn.state); // find or create the next batch
            obj.batchID = batchId;
        }

        return obj.id;
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