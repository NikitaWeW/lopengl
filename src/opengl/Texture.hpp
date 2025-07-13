#pragma once
#include "Object.hpp"
#include <string>
#include <filesystem>
#include "glad/gl.h"

namespace ogl
{
    class Texture : public Object
    {
    public:
        std::string type = "";
        Texture() = default;
        explicit Texture(GLenum filtermin, GLenum filtermag, GLenum wrap = GL_CLAMP_TO_EDGE) noexcept;
        explicit Texture(std::filesystem::path const &filepath, bool flip = false, bool srgb = false, std::string const &type = "", bool *isGrayScalePtr = nullptr);
        ~Texture();

        void bind(unsigned slot = 0) const noexcept override;
    };
    class TextureMS : public Object
    {
    public:
        TextureMS() = default;
        explicit TextureMS(GLenum filter, GLenum wrap = GL_CLAMP_TO_EDGE) noexcept;
        ~TextureMS();

        void bind(unsigned slot = 0) const noexcept override;
    };
    class Cubemap : public Object
    {
    public:
        Cubemap() = default;
        explicit Cubemap(unsigned) noexcept;
        // load equirectangular projection as a cube map
        explicit Cubemap(std::filesystem::path const &filepath, bool flip = false);
        void bind(unsigned slot = 0) const noexcept override;
    };
} // namespace ogl
