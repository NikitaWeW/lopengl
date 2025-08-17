#pragma once

#include "core/opengl/Shader.hpp"
#include "core/opengl/VertexBuffer.hpp"
#include "core/opengl/Texture.hpp"
#include "glm/glm.hpp"

namespace texture
{
    using Texture = ogl::Texture;
    Texture load(std::string_view path, std::string_view type);
} // namespace texture

namespace model
{
    struct Material
    {
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
        glm::vec3 transmittance;
        glm::vec3 emission;

        float shininess;
        float ior;
    };
    struct MaterialTextures
    {
        texture::Texture ambient;
        texture::Texture diffuse;
        texture::Texture specular;
        texture::Texture bump;
        texture::Texture displacement;
        texture::Texture alpha;
        texture::Texture reflection;
    };
    struct Mesh
    {
        Material material;
        MaterialTextures textures;
        ogl::VertexBuffer vbo;
        ogl::VertexArray vao;
        ogl::VertexBufferLayout vbLayout;
        unsigned count = 0;
    };
    
    class ILoader
    {
    public:
        virtual ~ILoader() = default;
        virtual Mesh load(std::string_view path) = 0;
    };
    
    class Loader
    {
    private:
        std::unordered_map<std::string, std::unique_ptr<ILoader>> m_loaders;
    public:
        Loader();
        void registerLoader(std::string_view extension, std::unique_ptr<ILoader> &&loader);
        Mesh load(std::string_view path);
    };
    
    class ObjLoader : public ILoader
    {
    public:
        Mesh load(std::string_view path) final;
    };
    class GltfLoader : public ILoader
    {
    public:
        Mesh load(std::string_view path) final;
    };
} // namespace model

