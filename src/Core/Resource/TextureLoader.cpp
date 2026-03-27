#include "Loaders.hpp"
#include "Core/Logging.hpp"
#include "stb_image.h"
#include "glm/gtc/integer.hpp"

TextureLoader::TextureLoader(ecs::registry &reg)
{ 
    mReg = &reg; 
}
ecs::entity TextureLoader::loadFromFile(std::string_view path, TextureLoaderOptions options)
{
    for(auto e_texture : mReg->view<Texture>())
        if(mReg->get<Texture>(e_texture).path == path)
            return e_texture;

    int width = 0, height = 0, numChannels = 0;
    stbi_set_flip_vertically_on_load(options.flip);
    float *buff = stbi_loadf(path.data(), &width, &height, &numChannels, 0);
    if(!buff)
    {
        LOG_ERROR("failed to load texture: \"{}\"!: {}", path, stbi_failure_reason());
        return INVALID_ENTITY;
    }
    assert(width > 0 && height > 0 && "failed to load a texture");
    Texture texture;
    texture.bitmap = Bitmap<float>(width, height, numChannels, buff);
    texture.path = path;
    // texture.numMipLevels = static_cast<uint32_t>(glm::ceil(glm::log2(std::max(texture.bitmap.size.x, texture.bitmap.size.y))));
    stbi_image_free(buff);

    return mReg->create(std::move(texture));
}
ecs::entity TextureLoader::loadFromMemory(void const *data, size_t size, TextureLoaderOptions options)
{
    int width = 0, height = 0, numChannels = 0;
    stbi_set_flip_vertically_on_load(options.flip);
    float *buff = stbi_loadf_from_memory(static_cast<stbi_uc const *>(data), size, &width, &height, &numChannels, 0);
    if(!buff)
    {
        LOG_ERROR("failed to load texture from memory: {}", stbi_failure_reason());
        return INVALID_ENTITY;
    }
    assert(width > 0 && height > 0 && "failed to load a texture");
    Texture texture;
    texture.bitmap = Bitmap<float>(width, height, numChannels, buff);
    texture.path = "loadFromMemory";
    // texture.numMipLevels = static_cast<uint32_t>(glm::ceil(glm::log2(std::max(texture.bitmap.size.x, texture.bitmap.size.y))));
    stbi_image_free(buff);

    return mReg->create(std::move(texture));
}

