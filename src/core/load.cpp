#include "load.hpp"
#include "tiny_obj_loader.h"
#include "tiny_gltf_loader.h"
#include "glm/glm.hpp"

#include <vector>
#include <iostream>

std::string_view getExtension(std::string_view path)
{
    if(path.find_last_of(".") != std::string::npos)
        return path.substr(path.find_last_of(".") + 1);
    return "";
}
texture::Texture loadTextureIfExists(std::string_view path, std::string_view type)
{
    if(path != "") 
        return texture::load(path, type);
    else 
        return texture::Texture{};
}

static model::Mesh loadObjMesh(std::string_view path)
{
    tinyobj::ObjReaderConfig config;
    config.mtl_search_path = "./";
    tinyobj::ObjReader reader;

    if(!reader.ParseFromFile(std::string{path}, config)) {
        std::cout << "failed to load \"" << path << "\"\n";
        if(!reader.Error().empty()) {
            std::cout << reader.Error() << '\n';
        }
        return model::Mesh{
            .count = 0
        };
    }

    if(!reader.Warning().empty()) {
        std::cout << reader.Warning().c_str();
    }

    auto &attrib = reader.GetAttrib();
    auto &shapes = reader.GetShapes();

    std::vector<glm::vec3> positions{};
    std::vector<glm::vec3> normals  {};
    std::vector<glm::vec2> texcoords{};
    std::vector<glm::vec3> tangents {};

    model::Mesh mesh{};
    mesh.count = 0;

    // "unzip" the object by unpacing the indices
    for(auto &shape : shapes) {
        size_t index_offset = 0;
        for(auto &face : shape.mesh.num_face_vertices) {
            for(size_t vertex = 0; vertex < face; ++vertex) {
                // access to vertex
                tinyobj::index_t idx = shape.mesh.indices[index_offset + vertex];
                assert(idx.texcoord_index >= 0);
                assert(idx.normal_index >= 0);

                positions.emplace_back(
                    attrib.vertices[3*size_t(idx.vertex_index)+0],
                    attrib.vertices[3*size_t(idx.vertex_index)+1],
                    attrib.vertices[3*size_t(idx.vertex_index)+2] 
                );
                normals.emplace_back(
                    attrib.normals[3*size_t(idx.normal_index)+0],
                    attrib.normals[3*size_t(idx.normal_index)+1],
                    attrib.normals[3*size_t(idx.normal_index)+2]
                );
                texcoords.emplace_back(
                    attrib.texcoords[2*size_t(idx.texcoord_index)+0],
                    attrib.texcoords[2*size_t(idx.texcoord_index)+1] 
                );

                ++mesh.count;
            }
            index_offset += face;
        }
    }

    auto &materials = reader.GetMaterials();

    if(!materials.empty())
    {
        auto const &material = materials[0];
        mesh.material = {
            .ambient = glm::vec3{material.ambient[0], material.ambient[1], material.ambient[2]},
            .diffuse = glm::vec3{material.diffuse[0], material.diffuse[1], material.diffuse[2]},
            .specular = glm::vec3{material.specular[0], material.specular[1], material.specular[2]},
            .transmittance = glm::vec3{material.transmittance[0], material.transmittance[1], material.transmittance[2]},
            .emission = glm::vec3{material.emission[0], material.emission[1], material.emission[2]},
            .shininess = material.shininess,
            .ior = material.ior
        };

        mesh.textures = {
            .ambient      = loadTextureIfExists(material.ambient_texname,      "ao"),
            .diffuse      = loadTextureIfExists(material.diffuse_texname,      "diffuse"),
            .specular     = loadTextureIfExists(material.specular_texname,     "specular"),
            .bump         = loadTextureIfExists(material.bump_texname,         "bump"),
            .displacement = loadTextureIfExists(material.displacement_texname, "displacement"),
            .alpha        = loadTextureIfExists(material.alpha_texname,        "alpha"),
            .reflection   = loadTextureIfExists(material.reflection_texname,   "reflection")
        };
    }

    assert(positions.size() == normals.size() && positions.size() == texcoords.size());

    // calculate tangents
    tangents.reserve(positions.size());
    for(size_t i = 0; i < positions.size(); i += 3)
    {
        glm::vec3 edge1 = positions[i+1] - positions[i+0];
        glm::vec3 edge2 = positions[i+2] - positions[i+0];
        glm::vec2 deltaUV1 = texcoords[i+1] - texcoords[i+0];
        glm::vec2 deltaUV2 = texcoords[i+2] - texcoords[i+0]; 

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        for(unsigned j = 0; j < 3; ++j)
        {
            tangents.emplace_back(
                f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
                f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
                f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)
            );
        }
    }

    // fill the vbo

    mesh.vbo = ogl::VertexBuffer{
        positions.size() * sizeof(decltype(positions[0])) +
        normals.size()   * sizeof(decltype(normals[0])) +
        texcoords.size() * sizeof(decltype(texcoords[0])) +
        tangents.size() * sizeof(decltype(tangents[0]))
    };

    glNamedBufferSubData(mesh.vbo.getRenderID(), 
        0, 
        positions.size() * sizeof(decltype(positions[0])), 
        positions.data()
    );
    glNamedBufferSubData(mesh.vbo.getRenderID(), 
        positions.size() * sizeof(decltype(positions[0])), 
        normals.size()   * sizeof(decltype(normals[0])),
        normals.data()
    );
    glNamedBufferSubData(mesh.vbo.getRenderID(), 
            positions.size() * sizeof(decltype(positions[0])) + 
            normals.size()   * sizeof(decltype(normals[0])), 
        texcoords.size() * sizeof(decltype(texcoords[0])),
        texcoords.data()
    );
    glNamedBufferSubData(mesh.vbo.getRenderID(), 
            positions.size() * sizeof(decltype(positions[0])) + 
            normals.size()   * sizeof(decltype(normals[0])) + 
            texcoords.size() * sizeof(decltype(texcoords[0])), 
        tangents.size() * sizeof(decltype(tangents[0])),
        tangents.data()
    );
    
    mesh.vbLayout = {
        /* 0. posiitons  */ { 3, GL_FLOAT, 0 },
        /* 1. normals    */ { 3, GL_FLOAT, positions.size() * sizeof(decltype(positions[0])) },
        /* 2. tex coords */ { 2, GL_FLOAT, positions.size() * sizeof(decltype(positions[0])) + normals.size() * sizeof(decltype(normals[0])) },
        /* 3. tangents   */ { 3, GL_FLOAT, positions.size() * sizeof(decltype(positions[0])) + normals.size() * sizeof(decltype(normals[0])) + texcoords.size() * sizeof(decltype(texcoords[0]))} 
    };

    mesh.vao = ogl::VertexArray{mesh.vbo, mesh.vbLayout};

    return mesh;
}
static model::Mesh loadGltfMesh(std::string_view path){
    assert(false && "gltf loading not implemented");
}

model::Mesh model::ObjLoader::load(std::string_view path)
{
    return loadObjMesh(path);
}
model::Mesh model::GltfLoader::load(std::string_view path)
{
    return loadGltfMesh(path);
}

model::Loader::Loader()
{
    registerLoader("obj",  std::make_unique<ObjLoader>());
    registerLoader("gltf", std::make_unique<ObjLoader>());
    registerLoader("glb",  std::make_unique<ObjLoader>());
}
void model::Loader::registerLoader(std::string_view extension, std::unique_ptr<ILoader> &&loader)
{
    m_loaders.emplace(extension, std::move(loader));
}
model::Mesh model::Loader::load(std::string_view path)
{
    std::string extension = std::string{getExtension(path)};

    if(m_loaders.find(extension) == m_loaders.end())
        throw std::invalid_argument{"extension \"" + extension + "\" not supported!"};
    if(!std::filesystem::exists(path))
        throw std::invalid_argument{"path \"" + std::string{path} + "\" doesent exist!"};

    return m_loaders.at(extension)->load(path);
}

texture::Texture texture::load(std::string_view path, std::string_view type)
{
    return ogl::Texture{path, false, type == "diffuse", std::string{type}};
}
