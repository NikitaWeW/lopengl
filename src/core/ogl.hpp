#pragma once
#include "glad/gl.h"
#include "equirect.hpp"
#include <string>
#include <vector>
#include <map>

namespace engine::detail::ogl
{
    /**
     * \brief An opengl object. Id 0 is invalid.
     */
    struct Object
    {
        GLuint id = 0;
    };
    struct Buffer : public Object
    {
        std::size_t size = 0;
        GLenum usage;
    };

    struct VBO : public Buffer {};
    struct IBO : public Buffer {};
    struct SSBO : public Buffer {};
    struct UBO : public Buffer {};
    
    struct Framebuffer : public Object {};
    struct VAO : public Object 
    {
        unsigned numVertexBuffers = 0;
    };
    
    struct Renderbuffer : public Object {
        unsigned width = 0;
        unsigned height = 0;
        unsigned numSamples = 1;
    };
    struct Texture : public Object {
        unsigned width = 0;
        unsigned height = 0;
        unsigned numSamples = 1;
    };
    struct Cubemap : public Object {
        unsigned width = 0;
        unsigned height = 0;
        unsigned numSamples = 1;
    };

    struct Program : public Object
    {
        struct Shader : public Object
        {
            GLenum type;
            std::string source;
        };

        mutable std::map<std::string_view, int> locationCache;
        std::vector<Shader> shaders;
        std::string dirpath;
    };

    Program compileShader(std::string_view dirpath);
    int getUniform(Program const &program, std::string_view name);
    int getUniformBlock(Program const &program, std::string_view name);
    int getStorageBlock(Program const &program, std::string_view name);

    bool isComplete(Framebuffer const &fbo);

    std::size_t getSizeOfGLType(GLenum type);

    Texture makeTexture(Bitmap<float> data, bool srgb);
    /**
     * Does not create buffer object if size is 0.
     */
    template<typename Buffer_t>
    inline Buffer_t makeBuffer(std::size_t size, void const *data = nullptr, GLenum usage = GL_DYNAMIC_DRAW)
    {
        Buffer_t buff;
        buff.size = size;
        buff.usage = usage;

        if(size != 0)
        {
            glCreateBuffers(1, &buff.id);
            glNamedBufferData(buff.id, size, data, usage);
        }
        return buff;
    }

    /** \copydoc makeBuffer */
    template<typename Buffer_t, typename T>
    inline Buffer_t makeBuffer(std::vector<T> const &data, GLenum usage = GL_DYNAMIC_DRAW)
    {
        return makeBuffer<Buffer_t>(data.size() * sizeof(T), data.data(), usage);
    }
} // namespace engine::detail::ogl

namespace ogl = engine::detail::ogl;
