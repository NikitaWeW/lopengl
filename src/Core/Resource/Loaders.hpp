// Source: github.com/nikitawew/breakout
#pragma once
#include "nicecs/ecs.hpp"
#include "Model.hpp"

struct TextureLoaderOptions
{
    bool flip = true; /// Flip the image vertically, so the first pixel in the output array is the bottom left.
};
struct ModelLoaderOptions
{
    bool flipWindingOrder = false; /// Flip the winding model of the triangles.
    bool flipUVs = false; /// Flip the texture coordinates vertically.
    TextureLoaderOptions textureOptions; /// Options for texture loading.
};

class ModelLoader
{
private:
    struct ModelLoaderImpl *mImpl;
public:
    /// @brief Construct an invalid loader.
    ModelLoader() = default;

    /// @brief Construct a valid loader.
    explicit ModelLoader(ecs::registry &reg);

    /// @brief Load a model from file.
    /// @param path The path to the file 
    /// @param options The options for loading the model.
    ecs::entity loadFromFile(std::string_view path, ModelLoaderOptions options = {});

    /// @brief Load a model from bytes.
    /// @param data The pointer to the model data.
    /// @param size The size of @p data.
    /// @param options The options for loading the model.
    ecs::entity loadFromMemory(void const *data, size_t size, ModelLoaderOptions options = {});

    /// @brief Get the default material.
    /// This material may be (partially) applied to meshes without some parameters or textures.
    /// Is guaranteed to have all the parameters and textures set.
    Material getDefaultMaterial() const;
};

class TextureLoader
{
private:
    ecs::registry *mReg;
public:
    /// @brief Construct an invalid loader.
    TextureLoader() = default;

    /// @brief Construct a valid loader.
    explicit TextureLoader(ecs::registry &reg);

    /// @brief Load a texture from file.
    /// @param path The path to the file 
    /// @param options The options for loading the texture.
    ecs::entity loadFromFile(std::string_view path, TextureLoaderOptions options = {});

    /// @brief Load a texture from bytes.
    /// @param data The pointer to the texture data.
    /// @param size The size of @p data.
    /// @param options The options for loading the texture.
    ecs::entity loadFromMemory(void const *data, size_t size, TextureLoaderOptions options = {});
};
