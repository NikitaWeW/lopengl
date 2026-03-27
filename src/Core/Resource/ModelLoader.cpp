// Source: github.com/nikitawew/breakout
#include "Model.hpp" 
#include "Loaders.hpp"
#include "Core/Logging.hpp"
#include <filesystem>
#include <glm/gtc/quaternion.hpp>

#include "meshoptimizer.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "stb_image.h"

// Enable verbose loading by uncommenting the line below or adding a compiler definition.
#define MODEL_LOADER_TRACE(...) LOG_TRACE(__VA_ARGS__)
#ifndef MODEL_LOADER_TRACE
#define MODEL_LOADER_TRACE(...)
#endif

constexpr glm::mat4 toMat4(aiMatrix4x4 const &from)
{
    glm::mat4 to{};
    //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}
constexpr glm::vec3 toVec3(aiVector3D const &aivector)
{
    return glm::vec3{(float) aivector.x, (float) aivector.y, (float) aivector.z};
}
constexpr glm::vec3 toVec3(aiColor3D const &aivector)
{
    return glm::vec3{(float) aivector.r, (float) aivector.g, (float) aivector.b};
}
constexpr glm::vec2 toVec2(aiVector2D const &aivector)
{
    return glm::vec2((float) aivector.x, (float) aivector.y);
}
constexpr glm::quat toQuat(aiQuaternion const &aiquaternion)
{
    return glm::quat{aiquaternion.w, aiquaternion.x, aiquaternion.y, aiquaternion.z};
}
static float u8ToFloat(unsigned char v) { return v / 255.0f; }
static glm::vec3 getColor(aiMaterial const *material, glm::vec3 defaultColor, const char* key, unsigned int type, unsigned int idx)
{
    aiColor3D color;
    if(material->Get(key, type, idx, color) == AI_SUCCESS) {
        return {color.r, color.g, color.b};
    }

    return defaultColor;
}
static glm::vec4 getColor(aiMaterial const *material, glm::vec4 defaultColor, const char* key, unsigned int type, unsigned int idx)
{
    aiColor4D color;
    if(material->Get(key, type, idx, color) == AI_SUCCESS) {
        return {color.r, color.g, color.b, color.a};
    }

    return defaultColor;
}
static float getColor(aiMaterial const *material, float defaultColor, const char* key, unsigned int type, unsigned int idx)
{
    float color;
    if(material->Get(key, type, idx, color) == AI_SUCCESS) {
        return color;
    }

    return defaultColor;
}

static void setMissingTextures(Material::Textures &material, Material::Textures const &defaultMaterial)
{
    if(!material.albedo      ) material.albedo       = defaultMaterial.albedo;
    if(!material.metallic    ) material.metallic     = defaultMaterial.metallic;
    if(!material.roughness   ) material.roughness    = defaultMaterial.roughness;
    if(!material.ambient     ) material.ambient      = defaultMaterial.ambient;
    if(!material.normal      ) material.normal       = defaultMaterial.normal;
    if(!material.displacement) material.displacement = defaultMaterial.displacement;
}
static aiNodeAnim const *findNodeAnim(aiAnimation const *animation, std::string_view nodeName)
{
    for(unsigned i = 0; i < animation->mNumChannels; ++i)
    {
        aiNodeAnim const *node = animation->mChannels[i];
        if(std::string_view{node->mNodeName.C_Str()} == nodeName)
            return node;
    }

    return nullptr;
}

// FIXME: probably wrong.
static void calculateMissingPrimitives(Mesh &mesh)
{
    assert(!mesh.geometry.positions.empty());

    bool indexed = !mesh.geometry.indices.empty();
    if(mesh.geometry.texCoords.empty())
    {
        MODEL_LOADER_TRACE("Calculating missing texcoords");
        mesh.geometry.texCoords.resize(mesh.geometry.positions.size());
        for(size_t i = 0; i < (indexed ? mesh.geometry.indices.size() : mesh.geometry.positions.size()); ++i)
        {
            unsigned index = indexed ? mesh.geometry.indices[i] : i;

            mesh.geometry.texCoords[index] = std::array<glm::vec2, 6>{
                glm::vec2{0, 0},
                glm::vec2{0, 1},
                glm::vec2{1, 1},
                glm::vec2{1, 0},
                glm::vec2{1, 1},
                glm::vec2{0, 0},
            }[i%6];
        }
    }

    if(mesh.geometry.normals.empty())
    {
        MODEL_LOADER_TRACE("Calculating missing normals");
        mesh.geometry.normals.resize(mesh.geometry.positions.size());
        for(size_t i = 0; i < (indexed ? mesh.geometry.indices.size() : mesh.geometry.positions.size()); i+=3)
        {
            size_t i0 = indexed ? mesh.geometry.indices[i+0] : i+0;
            size_t i1 = indexed ? mesh.geometry.indices[i+1] : i+1;
            size_t i2 = indexed ? mesh.geometry.indices[i+2] : i+2;

            glm::vec3 e1 = mesh.geometry.positions[i1] - mesh.geometry.positions[i0];
            glm::vec3 e2 = mesh.geometry.positions[i2] - mesh.geometry.positions[i0];
            glm::vec3 normal = glm::normalize(glm::cross(e1, e2));
            mesh.geometry.normals[i0] = normal;
            mesh.geometry.normals[i1] = normal;
            mesh.geometry.normals[i2] = normal;
        }
    }

    if(mesh.geometry.tangents.empty())
    {
        MODEL_LOADER_TRACE("Calculating missing tangents");
        mesh.geometry.tangents.resize(mesh.geometry.positions.size());
        for(size_t i = 0; i < (indexed ? mesh.geometry.indices.size() : mesh.geometry.positions.size()); i+=3)
        {
            size_t i0 = indexed ? mesh.geometry.indices[i+0] : i+0;
            size_t i1 = indexed ? mesh.geometry.indices[i+1] : i+1;
            size_t i2 = indexed ? mesh.geometry.indices[i+2] : i+2;

            glm::vec3 edge1 = mesh.geometry.positions[i1] - mesh.geometry.positions[i0];
            glm::vec3 edge2 = mesh.geometry.positions[i2] - mesh.geometry.positions[i0];
            glm::vec2 deltaUV1 = mesh.geometry.texCoords[i1] - mesh.geometry.texCoords[i0];
            glm::vec2 deltaUV2 = mesh.geometry.texCoords[i2] - mesh.geometry.texCoords[i0];

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
            glm::vec3 tangent = {
                f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
                f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
                f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z),
            };
            glm::vec3 normal = mesh.geometry.normals[i0];
            tangent = glm::normalize(tangent - normal * glm::dot(normal, tangent));

            mesh.geometry.tangents[i0] = tangent;
            mesh.geometry.tangents[i1] = tangent;
            mesh.geometry.tangents[i2] = tangent;
        }
    }
}
static void optimizeMesh(Mesh &mesh)
{
    Mesh oldMesh = mesh;
    bool indexed = !oldMesh.geometry.indices.empty();

    size_t index_count = indexed ? oldMesh.geometry.indices.size() : oldMesh.geometry.positions.size();
    size_t vertex_count = indexed ? oldMesh.geometry.positions.size() : index_count;
    std::vector<meshopt_Stream> streams = {
        meshopt_Stream{oldMesh.geometry.positions.data(), sizeof(glm::vec3), sizeof(glm::vec3)},
        meshopt_Stream{oldMesh.geometry.texCoords.data(), sizeof(glm::vec2), sizeof(glm::vec2)},
        meshopt_Stream{oldMesh.geometry.normals  .data(), sizeof(glm::vec3), sizeof(glm::vec3)},
        meshopt_Stream{oldMesh.geometry.tangents .data(), sizeof(glm::vec3), sizeof(glm::vec3)}
    };

    if(!oldMesh.geometry.boneIDs.empty())
    {
        streams.emplace_back(meshopt_Stream{oldMesh.geometry.boneIDs.data(), sizeof(glm::ivec4), sizeof(glm::ivec4)});
        streams.emplace_back(meshopt_Stream{oldMesh.geometry.weights.data(), sizeof(glm::vec4),  sizeof(glm::vec4)});
    }

    std::vector<unsigned int> remap(vertex_count);
    size_t new_vertex_count = meshopt_generateVertexRemapMulti(remap.data(), indexed ? oldMesh.geometry.indices.data() : nullptr, index_count, vertex_count, streams.data(), streams.size());
    mesh.geometry.indices.resize(index_count); meshopt_remapIndexBuffer(mesh.geometry.indices.data(), indexed ? oldMesh.geometry.indices.data() : nullptr, index_count, remap.data());
    mesh.geometry.positions.resize(new_vertex_count); meshopt_remapVertexBuffer(mesh.geometry.positions.data(), streams[0].data, vertex_count, streams[0].size, remap.data());
    mesh.geometry.texCoords.resize(new_vertex_count); meshopt_remapVertexBuffer(mesh.geometry.texCoords.data(), streams[1].data, vertex_count, streams[1].size, remap.data());
    mesh.geometry.normals  .resize(new_vertex_count); meshopt_remapVertexBuffer(mesh.geometry.normals  .data(), streams[2].data, vertex_count, streams[2].size, remap.data());
    mesh.geometry.tangents .resize(new_vertex_count); meshopt_remapVertexBuffer(mesh.geometry.tangents .data(), streams[3].data, vertex_count, streams[3].size, remap.data());
    if(!oldMesh.geometry.boneIDs.empty())
    {
        mesh.geometry.boneIDs  .resize(new_vertex_count); meshopt_remapVertexBuffer(mesh.geometry.boneIDs  .data(), streams[4].data, vertex_count, streams[4].size, remap.data());
        mesh.geometry.weights  .resize(new_vertex_count); meshopt_remapVertexBuffer(mesh.geometry.weights  .data(), streams[5].data, vertex_count, streams[5].size, remap.data());
    }

    if(oldMesh.geometry.indices.size() == mesh.geometry.indices.size() && oldMesh.geometry.positions.size() == mesh.geometry.positions.size())
        MODEL_LOADER_TRACE("Optimized mesh. Nothing changed.");
    else
        MODEL_LOADER_TRACE("Optimized mesh. Had {} indices and {} vertices. Has {} indices and {} vertices", oldMesh.geometry.indices.size(), oldMesh.geometry.positions.size(), mesh.geometry.indices.size(), mesh.geometry.positions.size());
}
static void moveMesh(Mesh::Geometry &primitives, glm::mat4 const &mat)
{
    if(mat == glm::mat4{1.0f})
        return;

    MODEL_LOADER_TRACE("Applying transformation to a mesh.");

    glm::mat4 normalMat = glm::inverse(glm::transpose(mat));

    for(auto &position : primitives.positions)
        position = mat * glm::vec4(position, 1);
    for(auto &normal : primitives.normals)
        normal = normalMat * glm::vec4(normal, 0);
    for(auto &tangent : primitives.tangents)
        tangent = normalMat * glm::vec4(tangent, 0);
}

static void extractVertexData(aiMesh const *aimesh, Mesh &mesh)
{
    for(unsigned i = 0; i < aimesh->mNumVertices; ++i) {
        mesh.geometry.positions.emplace_back(aimesh->mVertices[i].x, aimesh->mVertices[i].y, aimesh->mVertices[i].z);
        if(aimesh->HasNormals())
            mesh.geometry.normals.emplace_back(aimesh->mNormals[i].x, aimesh->mNormals[i].y, aimesh->mNormals[i].z);
        if(aimesh->HasTangentsAndBitangents())
            mesh.geometry.tangents.emplace_back(aimesh->mTangents[i].x, aimesh->mTangents[i].y, aimesh->mTangents[i].z);
        if(aimesh->HasTextureCoords(0))
            mesh.geometry.texCoords.emplace_back(aimesh->mTextureCoords[0][i].x, aimesh->mTextureCoords[0][i].y);
    }
    for(unsigned i = 0; i < aimesh->mNumFaces; ++i) {
        aiFace face = aimesh->mFaces[i];
        for(unsigned j = 0; j < face.mNumIndices; ++j) {
            mesh.geometry.indices.push_back(face.mIndices[j]);
        }
    }
}
static void extractBoneData(aiMesh const *aimesh, Mesh &mesh, Model::Skeleton &skeleton)
{
    // i hate it -- april 2025
    // it works -- october 2025
    glm::ivec4 boneIDs{-1}; mesh.geometry.boneIDs.resize(mesh.geometry.positions.size(), boneIDs);
    glm::vec4 weights{0}; mesh.geometry.weights.resize(mesh.geometry.positions.size(), weights);
    for(unsigned boneIndex = 0; boneIndex < aimesh->mNumBones; ++boneIndex) {
        int boneID = -1;
        aiBone const *bone = aimesh->mBones[boneIndex];
        std::string boneName = bone->mName.C_Str();
        if(skeleton.boneMap.find(boneName) == skeleton.boneMap.end())
        {
            unsigned id = skeleton.boneMap.size();
            skeleton.bindTransform.emplace_back(toMat4(bone->mOffsetMatrix));
            skeleton.boneMap.try_emplace(boneName, id);
            boneID = id;
        } else
        {
            boneID = skeleton.boneMap.at(boneName);
        }
        assert(boneID != -1);

        for(unsigned weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex)
        {
            unsigned vertexID = bone->mWeights[weightIndex].mVertexId;
            assert(vertexID < mesh.geometry.positions.size());
            // record it in the first uninitialized slot
            for(unsigned i = 0; i < 4; ++i)
            {
                if(mesh.geometry.boneIDs[vertexID][i] == -1)
                {
                    mesh.geometry.boneIDs[vertexID][i] = boneID;
                    mesh.geometry.weights[vertexID][i] = bone->mWeights[weightIndex].mWeight;
                    break;
                }
            }
        }
    }
}
static void normalizeWeights(Mesh::Geometry &geometry)
{
    for(auto &weight : geometry.weights)
        if(weight[0] + weight[1] + weight[2] + weight[3] > 1e-6f)
            weight /= weight[0] + weight[1] + weight[2] + weight[3];
}

struct ModelLoaderImpl
{
    Material mDefaultMaterial;
    ModelLoaderOptions mOptions;
    TextureLoader mTextureLoader;

    aiScene const *mScene = nullptr;
    Model *mModel = nullptr;
    ecs::registry *mRegistry = nullptr;

    // === === === ===
    ModelLoaderImpl(ecs::registry &reg);
    ecs::entity fromRawAssimpTexture(aiTexture const *texture);
    void loadMaterialTexture(aiMaterial const *material, aiTextureType const type, ecs::entity &out);
    Material convertMaterial(aiMaterial const *aimaterial, Material::Properties const &defaultProperties);
    Mesh processMesh(aiMesh const *aimesh, glm::mat4 const &transform);
    ecs::entity load();
    void processNodeMeshes(aiNode const *node, glm::mat4 parentTransform = glm::mat4{1.0f});
    Animation processAnimation(aiAnimation const *animation);
    ecs::entity processLight(aiLight const *light);
};

ModelLoaderImpl::ModelLoaderImpl(ecs::registry &reg)
{
    mRegistry = &reg;
    ecs::entity white = 0;
    ecs::entity normal = 0;
    ecs::entity black = 0;
    ecs::entity tile = 0;

    for(ecs::entity e_texture : mRegistry->view<Texture>())
    {
        auto &texture = mRegistry->get<Texture>(e_texture);

        if(texture.path == "default/white")
            white = e_texture;
        if(texture.path == "default/normal")
            normal = e_texture;
        if(texture.path == "default/black")
            black = e_texture;
        if(texture.path == "default/tile")
            tile = e_texture;
    }

    if(!white)
        white = mRegistry->create(Texture{
            .bitmap = Bitmap<float>{1, 1, 3, std::array<float, 1*3>{
                1, 1, 1
            }.data()},
            .path = "default/white"
        });
    if(!normal)
        normal = mRegistry->create(Texture{
            .bitmap = Bitmap<float>{1, 1, 3, std::array<float, 1*1*3>{
                0.5f, 0.5f, 1.0f
            }.data()},
            .path = "default/normal"
        });
    if(!black)
        black = mRegistry->create(Texture{
            .bitmap = Bitmap<float>{1, 1, 3, std::array<float, 1*3>{
                0, 0, 0
            }.data()},
            .path = "default/black"
        });
    if(!tile)
        tile = mRegistry->create(Texture{
            .bitmap = Bitmap<float>{8, 8, 3, std::array<float, 8*8*3>{ // checkerboard
                1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 
                0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 
                1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 
                0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 
                1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 
                0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 
                1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 
                0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 
            }.data()},
            .path = "default/tile"
        });

    mDefaultMaterial = {
        .textures = {
            .albedo       = tile,
            .metallic     = black,
            .roughness    = white,
            .ambient      = white,
            .normal       = normal,
            .displacement = black,
        },
        .properties = {
            .ambient       = {0.1f, 0.1f, 0.1f},
            .albedo        = {0.8f, 0.8f, 0.8f, 1.0f},
            .specular      = {0.5f, 0.5f, 0.5f},
            .emission      = {0.0f, 0.0f, 0.0f},

            .shininess = 32.0f,
            .metallic = 0.0f,
            .ior       = 1.5f
        }
    };
}

Material ModelLoader::getDefaultMaterial() const
{
    assert(mImpl && "Invalid loader! (make sure to not use default constructor when making an actual loader)");
    if(!mImpl)
    {
        LOG_ERROR("Invalid model loader! (make sure to not use default constructor when making an actual loader)");
        return {};
    }

    return mImpl->mDefaultMaterial;
}
ecs::entity ModelLoaderImpl::fromRawAssimpTexture(aiTexture const *texture)
{
    assert(texture->mHeight == 0);
    unsigned const width = static_cast<unsigned>(texture->mWidth);
    unsigned const height = static_cast<unsigned>(texture->mHeight);

    if(!texture->pcData)
    {
        LOG_ERROR("aiTexture has no texel data");
        return INVALID_ENTITY;
    }

    Texture result;
    result.bitmap = Bitmap<float>(width, height, 4);
    // result.path = "raw resource " + stringID() + " aiTexture " + texture->mFilename.C_Str();

    for (size_t i = 0; i < width * height; ++i)
    {
        aiTexel const &texel = texture->pcData[i];
        glm::uvec2 pixel{static_cast<float>(i % width), static_cast<unsigned>(i / width)};
        result.bitmap.setPixel(pixel.x, pixel.y, {
            u8ToFloat(texel.r),
            u8ToFloat(texel.g),
            u8ToFloat(texel.b),
            u8ToFloat(texel.a)
        });
    }

    return mRegistry->create(std::move(result));
}
void ModelLoaderImpl::loadMaterialTexture(aiMaterial const *material, aiTextureType const type, ecs::entity &out)
{
    if(material->GetTextureCount(type) == 0 || out)
        return;

    // load the first texture (multiple texture of the same type are not supported)
    std::string directory = std::filesystem::path{mModel->path}.parent_path().string();
    aiString str;
    if(material->GetTexture(type, 0, &str) != AI_SUCCESS)
        return;
    bool srgb = type == aiTextureType_DIFFUSE;

    aiTexture const *embedded = mScene->GetEmbeddedTexture(str.C_Str());
    if(embedded)
    {
        if(embedded->mHeight == 0)
        {
            MODEL_LOADER_TRACE("Loading embedded compressed texture \"{}\"", embedded->mFilename.C_Str());
            out = mTextureLoader.loadFromMemory(embedded->pcData, embedded->mWidth, mOptions.textureOptions);
        } else
        {
            MODEL_LOADER_TRACE("Loading embedded raw texture \"{}\"", embedded->mFilename.C_Str());
            out = fromRawAssimpTexture(embedded);
        }
    }
    else
    {
        std::string filepath = directory + '/' + str.C_Str();
    
        MODEL_LOADER_TRACE("Loading file texture \"{}\"", filepath);
        out = mTextureLoader.loadFromFile(filepath, mOptions.textureOptions);
    }

    if(out)
    {
        auto &tex = mRegistry->get<Texture>(out);
        tex.srgb = srgb;
    }
}
Material ModelLoaderImpl::convertMaterial(aiMaterial const *aimaterial, Material::Properties const &defaultProperties)
{
    Material material;

    // https://github.com/assimp/assimp/issues/430
    loadMaterialTexture(aimaterial, aiTextureType_DIFFUSE,           material.textures.albedo      );
    loadMaterialTexture(aimaterial, aiTextureType_NORMALS,           material.textures.normal      );
    loadMaterialTexture(aimaterial, aiTextureType_HEIGHT,            material.textures.normal      );
    loadMaterialTexture(aimaterial, aiTextureType_DISPLACEMENT,      material.textures.displacement);
    loadMaterialTexture(aimaterial, aiTextureType_AMBIENT_OCCLUSION, material.textures.ambient     );
    loadMaterialTexture(aimaterial, aiTextureType_DIFFUSE_ROUGHNESS, material.textures.roughness   );
    loadMaterialTexture(aimaterial, aiTextureType_METALNESS,         material.textures.metallic    );

    material.properties = {
        .ambient       = getColor(aimaterial, defaultProperties.ambient,       AI_MATKEY_COLOR_AMBIENT),
        .albedo        = getColor(aimaterial, defaultProperties.albedo,        AI_MATKEY_COLOR_DIFFUSE),
        .specular      = getColor(aimaterial, defaultProperties.specular,      AI_MATKEY_COLOR_SPECULAR),
        .emission      = getColor(aimaterial, defaultProperties.emission,      AI_MATKEY_COLOR_EMISSIVE),

        .shininess     = getColor(aimaterial, defaultProperties.shininess,     AI_MATKEY_SHININESS),
        .metallic      = getColor(aimaterial, defaultProperties.metallic,      AI_MATKEY_METALLIC_FACTOR),
        .ior           = getColor(aimaterial, defaultProperties.ior,           AI_MATKEY_REFRACTI)
    };

    return material;
}

Mesh ModelLoaderImpl::processMesh(aiMesh const *aimesh, glm::mat4 const &transform)
{
    MODEL_LOADER_TRACE("Loading mesh \"{}\"", aimesh->mName.C_Str());
    Mesh mesh;
    extractVertexData(aimesh, mesh);

    if(aimesh->HasBones())
    {
        extractBoneData(aimesh, mesh, mModel->skeleton);
        normalizeWeights(mesh.geometry);
    }

    if(!mScene->HasMaterials())
    {
        mesh.material = mDefaultMaterial;
    }
    else
    {
        aiMaterial const *aimaterial = mScene->mMaterials[aimesh->mMaterialIndex];
        mesh.material = convertMaterial(aimaterial, mDefaultMaterial.properties);
        setMissingTextures(mesh.material.textures, mDefaultMaterial.textures);
    }

    calculateMissingPrimitives(mesh);
    optimizeMesh(mesh);

    // Apply transformation only for meshes.
    // Models with bones should use the bone transformations.
    if(!aimesh->HasBones())
        moveMesh(mesh.geometry, transform);

    return mesh;
}
void ModelLoaderImpl::processNodeMeshes(aiNode const *node, glm::mat4 parentTransform)
{
    MODEL_LOADER_TRACE("Processing node \"{}\"", node->mName.C_Str());
    glm::mat4 nodeTransform = parentTransform * toMat4(node->mTransformation);
    for(unsigned i = 0; i < node->mNumMeshes; ++i) {
        mModel->meshes.emplace_back(processMesh(mScene->mMeshes[node->mMeshes[i]], nodeTransform));
    }
    for(unsigned i = 0; i < node->mNumChildren; ++i) {
        processNodeMeshes(node->mChildren[i], nodeTransform);
    }
}

void processAnimationNode(Animation &result, aiAnimation const *animation, Model::Skeleton const &skeleton, aiNode const *node)
{
    std::string nodeName = node->mName.C_Str();
    aiNodeAnim const *nodeAnim = findNodeAnim(animation, nodeName);

    if(nodeAnim && skeleton.boneMap.find(nodeName) != skeleton.boneMap.end()) {
        auto &keyframes = result.bones.at(skeleton.boneMap.at(nodeName));
        for(unsigned i = 0; i < nodeAnim->mNumPositionKeys; ++i)
        {
            auto const &key = nodeAnim->mPositionKeys[i];
            keyframes.positions.emplace_back(Animation::PositionKey{
                .value = toVec3(key.mValue),
                .timeTicks = static_cast<float>(key.mTime)
            });
        }
        for(unsigned i = 0; i < nodeAnim->mNumRotationKeys; ++i)
        {
            auto const &key = nodeAnim->mRotationKeys[i];
            keyframes.orientations.emplace_back(Animation::OrientationKey{
                .value = glm::normalize(toQuat(key.mValue)),
                .timeTicks = static_cast<float>(key.mTime)
            });
        }
        for(unsigned i = 0; i < nodeAnim->mNumScalingKeys; ++i)
        {
            auto const &key = nodeAnim->mScalingKeys[i];
            keyframes.scales.emplace_back(Animation::ScaleKey{
                .value = toVec3(key.mValue),
                .timeTicks = static_cast<float>(key.mTime)
            });
        }
    }

    for(unsigned i = 0; i < node->mNumChildren; ++i) {
        processAnimationNode(result, animation, skeleton, node->mChildren[i]);
    }
}
Animation ModelLoaderImpl::processAnimation(aiAnimation const *animation)
{
    MODEL_LOADER_TRACE("Processing animation \"{}\"", animation->mName.C_Str());

    Animation result;
    result.durationTicks = (float) animation->mDuration;
    result.ticksPerSecond = (animation->mTicksPerSecond > 0) ? (float) animation->mTicksPerSecond : 24.0f;
    result.name = animation->mName.C_Str();
    result.bones.resize(mModel->skeleton.boneMap.size());

    processAnimationNode(result, animation, mModel->skeleton, mScene->mRootNode);

    for(auto &bone : result.bones)
    {
        std::sort(bone.positions   .begin(), bone.positions   .end(), [](Animation::PositionKey    const &first, Animation::PositionKey    const &second){ return first.timeTicks < second.timeTicks; });
        std::sort(bone.orientations.begin(), bone.orientations.end(), [](Animation::OrientationKey const &first, Animation::OrientationKey const &second){ return first.timeTicks < second.timeTicks; });
        std::sort(bone.scales      .begin(), bone.scales      .end(), [](Animation::ScaleKey       const &first, Animation::ScaleKey       const &second){ return first.timeTicks < second.timeTicks; });
    }

    return result;
}

void calculateParent(Model::Skeleton &skeleton, aiNode const *node, int parent)
{
    std::string nodeName = node->mName.C_Str();

    if(skeleton.boneMap.find(nodeName) != skeleton.boneMap.end())
    {
        unsigned bone = skeleton.boneMap.at(nodeName);
        skeleton.parents.at(bone) = parent;
        skeleton.nodeTransform.at(bone) = toMat4(node->mTransformation);
        parent = bone;
    }

    for(unsigned i = 0; i < node->mNumChildren; ++i) {
        calculateParent(skeleton, node->mChildren[i], parent);
    }
}
ecs::entity ModelLoaderImpl::processLight(aiLight const *light)
{
    return 0;
    // switch(light->mType)
    // {
    // case aiLightSource_POINT:
    //     return mRegistry->create(
    //         PointLight{
    //             .color = toVec3(light->mColorDiffuse) * (1.0f / (light->mAttenuationQuadratic))
    //         },
    //         Transform{
    //             .position = toVec3(light->mPosition)
    //         }
    //     );
    // case aiLightSource_DIRECTIONAL:
    //     return mRegistry->create(
    //         DirectionalLight{
    //             .color = toVec3(light->mColorDiffuse) * (1.0f / (light->mAttenuationQuadratic))
    //         },
    //         Transform{
    //             .orientation = glm::quatLookAt(glm::normalize(toVec3(light->mDirection)), glm::normalize(toVec3(light->mUp)))
    //             // .orientation = glm::quatLookAt(toVec3(light->mDirection), glm::abs(glm::dot(toVec3(light->mDirection), glm::vec3{0, 1, 0})) > 0.999 ? glm::vec3{1, 0, 0} : glm::vec3{0, 1, 0})
    //         }
    //     );
    // case aiLightSource_SPOT:
    //     return mRegistry->create(
    //         SpotLight{
    //             .color = toVec3(light->mColorDiffuse) * (1.0f / (light->mAttenuationQuadratic)),
    //             .innerConeAngle = glm::degrees(light->mAngleInnerCone),
    //             .outerConeAngle = glm::degrees(light->mAngleOuterCone)
    //         },
    //         Transform{
    //             .position = toVec3(light->mPosition),
    //             .orientation = glm::quatLookAt(glm::normalize(toVec3(light->mDirection)), glm::normalize(toVec3(light->mUp)))
    //         }
    //     );
    // case aiLightSource_AREA:
    //     return mRegistry->create(
    //         AreaLight{
    //             .color = toVec3(light->mColorDiffuse) * (1.0f / (light->mAttenuationQuadratic)),
    //             .size = toVec2(light->mSize)
    //         },
    //         Transform{
    //             .position = toVec3(light->mPosition),
    //             .orientation = glm::quatLookAt(glm::normalize(toVec3(light->mDirection)), glm::normalize(toVec3(light->mUp)))
    //         }
    //     );
    // default:
    //     ENGINE_CORE_WARN("Unknown assimp light type: {}", (int) light->mType);
    //     return 0;
    // }
}

ecs::entity ModelLoaderImpl::load()
{
    if(mScene->HasMeshes())
        MODEL_LOADER_TRACE("Loading {} meshes.", mScene->mNumMeshes);
    if(mScene->HasAnimations())
        MODEL_LOADER_TRACE("Loading {} animations.", mScene->mNumAnimations);

    processNodeMeshes(mScene->mRootNode);

    MODEL_LOADER_TRACE("Model has {} bones.", mModel->skeleton.boneMap.size());

    mModel->skeleton.parents.resize(mModel->skeleton.boneMap.size());
    mModel->skeleton.nodeTransform.resize(mModel->skeleton.boneMap.size());
    calculateParent(mModel->skeleton, mScene->mRootNode, -1);

    for(unsigned i = 0; i < mScene->mNumAnimations; ++i)
    {
        mModel->animations.emplace_back(processAnimation(mScene->mAnimations[i]));
    }
    for(unsigned i = 0; i < mScene->mNumLights; ++i)
    {
        mModel->lights.emplace_back(processLight(mScene->mLights[i]));
    }

    // TODO: morph targets

    return mRegistry->create(std::move(*mModel));
    mModel = nullptr;
}

constexpr unsigned ASSIMP_FLAGS =
    aiProcess_SplitLargeMeshes      |
    aiProcess_GenNormals            |
    aiProcess_GenUVCoords           |
    aiProcess_FindInvalidData       |
    aiProcess_CalcTangentSpace      |
    aiProcess_Triangulate           |
    aiProcess_JoinIdenticalVertices |
    aiProcess_SortByPType           |
    aiProcess_OptimizeGraph         |
    aiProcess_OptimizeMeshes        |
    aiProcess_ValidateDataStructure |
    aiProcess_LimitBoneWeights;

ModelLoader::ModelLoader(ecs::registry &reg)
{ 
    mImpl = new ModelLoaderImpl{reg};
    mImpl->mTextureLoader = TextureLoader{*mImpl->mRegistry}; 
}
ecs::entity ModelLoader::loadFromFile(std::string_view path, ModelLoaderOptions options)
{
    assert(mImpl && "Invalid loader! (make sure to not use default constructor when making an actual loader)");
    for(auto e_model : mImpl->mRegistry->view<Model>())
        if(mImpl->mRegistry->get<Model>(e_model).path == path)
            return e_model;

    MODEL_LOADER_TRACE("---");
    MODEL_LOADER_TRACE("Loading model \"{}\"", path);
    Assimp::Importer importer;
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
    aiScene const *scene = importer.ReadFile(std::string{path},
        ASSIMP_FLAGS |
        (options.flipWindingOrder ? aiProcess_FlipWindingOrder : 0) |
        (options.flipUVs ? aiProcess_FlipUVs : 0)
    );

    if(!scene)
    {
        LOG_ERROR("Error parsing \"{}\": {}", path, importer.GetErrorString());
        return INVALID_ENTITY;
    }

    Model model;
    model.path = path;

    mImpl->mModel = &model;
    mImpl->mScene = scene;
    mImpl->mOptions = options;
    mImpl->mModel->skeleton.globalInverseTransform = glm::inverse(toMat4(mImpl->mScene->mRootNode->mTransformation));

    return mImpl->load();
}
ecs::entity ModelLoader::loadFromMemory(void const *data, size_t size, ModelLoaderOptions options)
{
    assert(mImpl && "Invalid loader! (make sure to not use default constructor when making an actual loader)");
    if(!mImpl)
    {
        LOG_ERROR("Invalid model loader! (make sure to not use default constructor when making an actual loader)");
        return 0;
    }
    MODEL_LOADER_TRACE("---");
    MODEL_LOADER_TRACE("Loading model from memory");
    Assimp::Importer importer;
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
    aiScene const *scene = importer.ReadFileFromMemory(data, size,
        ASSIMP_FLAGS |
        (options.flipWindingOrder ? aiProcess_FlipWindingOrder : 0) |
        (options.flipUVs ? aiProcess_FlipUVs : 0)
    );

    if(!scene)
    {
        LOG_ERROR("Error parsing from memory: {}", importer.GetErrorString());
        return INVALID_ENTITY;
    }

    Model model;
    model.path = "loadFromMemory";

    mImpl->mModel = &model;
    mImpl->mScene = scene;
    mImpl->mOptions = options;
    mImpl->mModel->skeleton.globalInverseTransform = glm::inverse(toMat4(mImpl->mScene->mRootNode->mTransformation));

    return mImpl->load();
}
