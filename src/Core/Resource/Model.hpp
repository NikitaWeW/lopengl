#pragma once
#include "Core/DSA/ECS.hpp"
#include "Core/DSA/Bitmap.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <vector>
#include <string>

struct Texture
{
    Bitmap<float> bitmap;
    bool srgb = false;
    unsigned numMipLevels = 1;
    std::string path;
};

struct Material
{
    /// @brief Contains entities with the Texture component, invalid if not present.
    struct Textures
    {
        ecs::entity albedo = 0;
        ecs::entity metallic = 0;
        ecs::entity roughness = 0;
        ecs::entity ambient = 0;
        ecs::entity normal = 0;
        ecs::entity displacement = 0;
    } textures;
    struct Properties
    {
        glm::vec3 ambient;
        glm::vec4 albedo;
        glm::vec3 specular;
        glm::vec3 emission;

        float shininess;
        float metallic;
        float ior;
    } properties;
};
struct Animation
{
    struct PositionKey
    {
        glm::vec3 value;
        float timeTicks;
    };
    struct OrientationKey
    {
        glm::quat value;
        float timeTicks;
    };
    struct ScaleKey
    {
        glm::vec3 value;
        float timeTicks;
    };
    struct Keyframes
    {
        std::vector<PositionKey   > positions;
        std::vector<OrientationKey> orientations;
        std::vector<ScaleKey      > scales;
    };

    std::vector<Keyframes> bones;
    std::string name = "";
    float durationTicks = 0;
    float ticksPerSecond = 0;
};
struct Mesh
{
    struct Geometry
    {
        // guaranteed
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec3> tangents;
        std::vector<unsigned> indices;

        // optional
        // hope 4 bones per vertex would be enough
        std::vector<glm::vec4> boneIDs;
        std::vector<glm::vec4> weights;
    } geometry;
    
    Material material;
};
struct Model
{
    std::vector<Mesh> meshes;
    std::vector<Animation> animations;
    std::string path;

    std::vector<ecs::entity> lights;

    struct Skeleton
    {
        glm::mat4 globalInverseTransform;
        std::vector<glm::mat4> bindTransform;
        std::vector<glm::mat4> nodeTransform;
        std::vector<int> parents; // -1 if root
        std::unordered_map<std::string, unsigned> boneMap; // bone name to bone id
    } skeleton;
};
