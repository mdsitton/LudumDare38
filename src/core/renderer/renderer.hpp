#pragma once
#include <memory>
#include <unordered_map>
#include <map>
#include <string>

#include <spdlog/spdlog.h>

#include "texture.hpp"
#include "batch.hpp"
#include "mesh.hpp"

namespace ORCore
{

    std::vector<Vertex> create_rect_mesh(glm::vec4 color);

    void set_state(std::map<RenderState, int>& state, RenderState stateItem, int value);

    struct RenderObject
    {
        Mesh mesh;
        glm::mat4 modelMatrix;
        std::map<RenderState, int> state;
        int id; // id of this object in the renderer.
        int batchID;
        RenderObject();
        void set_state(RenderState stateItem, int value);
        void set_scale(glm::vec3&& scale);
        void set_translation(glm::vec3&& translation);
        void set_primitive_type(Primitive primitive);
        void set_geometry(std::vector<Vertex>&& geometry);
        void set_texture(int _texture);
        void set_program(int _program);
        void set_point_size(int pointSize);
        void update();
    };

    // Builds and renders batches from objects.
    class Renderer
    {
    public:
        Renderer();
        void init_gl();
        void clear_object_batch(int objID);
        int add_object_dedibatch(const RenderObject& objIn);
        int add_object(const RenderObject& objIn);
        int readd_object(int objID);
        RenderObject* get_object(int objID);
        void update_object(int objID);
        int add_texture(Image&& img);
        int add_program(Shader&& vertex, Shader&& fragment);
        void set_camera_transform(std::string name, glm::mat4&& transform);
        // add global attribute/uniforms for shaders ?
        void commit();
        void render();
        void clear();
        ~Renderer();

    private:
        int create_batch(const std::map<RenderState, int>& batchState, int batchSize);
        int find_batch(const std::map<RenderState, int>& batchState);
        std::vector<RenderObject> m_objects;
        std::vector<std::unique_ptr<Batch>> m_batches;
        std::unordered_map<std::string, glm::mat4> m_cameraUniforms;
        std::vector<std::unique_ptr<Texture>> m_textures;
        std::vector<std::unique_ptr<ShaderProgram>> m_programs;
        std::shared_ptr<spdlog::logger> m_logger;
        int m_defaultTextureID;
    };
}