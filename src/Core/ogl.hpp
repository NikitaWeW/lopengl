#pragma once
#include "glad/gl.h"
#include "DSA/Bitmap.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace ogl
{
/// @brief Generic OpenGL object wrapper.
/// Holds an OpenGL object id.
struct Object
{
    /// @brief OpenGL object id (0 = invalid).
    GLuint id = 0;
};

/// @brief Generic buffer object.
/// Represents an OpenGL buffer with a size in bytes and a usage hint.
struct Buffer : public Object
{
    /// @brief Size of the buffer in bytes.
    std::size_t size = 0;
    /// @brief Usage hint passed to OpenGL (e.g., GL_STATIC_DRAW).
    GLenum usage;
};

/// @brief Vertex Buffer Object wrapper.
struct VBO : public Buffer {};
/// @brief Index Buffer Object wrapper.
struct IBO : public Buffer {};
/// @brief Shader Storage Buffer Object wrapper.
struct SSBO : public Buffer {};
/// @brief Uniform Buffer Object wrapper.
struct UBO : public Buffer {};

/// @brief Framebuffer wrapper.
struct Framebuffer : public Object {};

/// @brief Vertex Array Object wrapper.
struct VAO : public Object 
{
    /// @brief Number of vertex buffers attached to this VAO.
    std::vector<ogl::Buffer> buffers;
};

/// @brief Renderbuffer wrapper.
/// Stores dimensions and sample count for multisampled renderbuffers.
struct Renderbuffer : public Object {
    /// @brief Width in pixels.
    unsigned width = 0;
    /// @brief Height in pixels.
    unsigned height = 0;
    /// @brief Number of samples for multisampling (1 = no multisampling).
    unsigned numSamples = 1;
};

/// @brief Texture wrapper.
/// Stores texture dimensions and sample count for multisampled textures.
struct Texture : public Object {
    /// @brief Width in pixels.
    unsigned width = 0;
    /// @brief Height in pixels.
    unsigned height = 0;
    /// @brief Number of samples for multisampled textures (1 = no multisampling).
    unsigned numSamples = 1;
};

/// @brief Cubemap wrapper.
/// Stores cubemap face dimensions and sample count.
struct Cubemap : public Object {
    /// @brief Width of each cubemap face in pixels.
    unsigned width = 0;
    /// @brief Height of each cubemap face in pixels.
    unsigned height = 0;
    /// @brief Number of samples for multisampled cubemaps (1 = no multisampling).
    unsigned numSamples = 1;
};

/// @brief Program wrapper containing shaders and reflection cache.
/// Holds compiled/linked program id, attached shader sources, and a cache for uniform locations.
struct Program : public Object
{
    /// @brief Shader description used during program creation.
    struct Shader : public Object
    {
        /// @brief Shader type (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc.).
        GLenum type;
        /// @brief Shader source code.
        std::string source;
        /// @brief The path to the source file.
        std::string path;
    };

    /// @brief Cache mapping uniform name to location for faster lookups.
    mutable std::unordered_map<std::string, int> locationCache;
    /// @brief Attached shaders that will be compiled/linked into the program.
    std::vector<Shader> shaders;
    /// @brief Directory path used to resolve shader includes or files.
    std::string dirpath;
};

/// @brief Compile and link a shader program from a directory of shader sources.
/// @param dirpath Directory containing shader source files.
/// @return A Program object with compiled and linked shaders (id == 0 on failure).
Program compileShader(std::string_view dirpath);

/// @brief Delete the old program if valid, compile and link a shader program from a directory of shader sources.
/// @param dirpath Directory containing shader source files.
/// @return True if compilation was successful, false otherwise.
bool recompileShader(Program &program, std::string_view dirpath);

/// @brief Get the location of a uniform variable in a program.
/// @param program The program to query.
/// @param name The uniform variable name.
/// @return The uniform location, or -1 if not found.
int getUniform(Program const &program, std::string_view name);

/// @brief Get the index of a uniform block in a program.
/// @param program The program to query.
/// @param name The uniform block name.
/// @return The uniform block index, or -1 if not found.
int getUniformBlock(Program const &program, std::string_view name);

/// @brief Get the index of a shader storage block in a program.
/// @param program The program to query.
/// @param name The storage block name.
/// @return The storage block index, or -1 if not found.
int getStorageBlock(Program const &program, std::string_view name);

/// @brief Check whether a framebuffer is complete.
/// @param fbo The framebuffer to check.
/// @return True if the framebuffer is complete and ready for rendering.
bool isComplete(Framebuffer const &fbo);

/// @brief Return the size in bytes of a given GL type enum.
/// Handles scalar, vector, matrix, half/fixed and common packed formats.
/// @param type The GLenum representing the GL type.
/// @return Size in bytes for the provided type.
std::size_t getSizeOfGLType(GLenum type);

/// @brief Create a 2D texture from floating point bitmap data.
/// @param data Bitmap containing float pixel data (assumed linear unless \p srgb is true).
/// @param srgb If true, create the texture with sRGB sampling/format where applicable.
/// @return A Texture object representing the created OpenGL texture.
Texture makeTexture(Bitmap<float> const &data, bool srgb);

/// @brief Create a cubemap texture from six floating point bitmaps.
/// @param data Array of 6 bitmaps corresponding to cubemap faces.
/// @return A Cubemap object representing the created OpenGL cubemap.
Cubemap makeCubemap(std::array<Bitmap<float>, 6> const &data);

/// @brief Create a buffer object.
/// Creates and initializes an OpenGL buffer object of the requested size. If \p size is 0,
/// no OpenGL buffer is created and a Buffer_t with id == 0 is returned. The created buffer's \c size
/// @tparam Buffer_t The concrete buffer type to create (e.g., VBO, IBO, SSBO, UBO).
/// @param size Size in bytes for the buffer to allocate.
/// @param data Optional pointer to initial data to upload into the buffer. May be nullptr.
/// @param usage Usage hint passed to OpenGL (e.g., GL_STATIC_DRAW, GL_DYNAMIC_DRAW).
/// @return A Buffer_t instance representing the created buffer. If \p size is 0, returns an uncreated buffer (id == 0).
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

/// @copydoc makeBuffer.
template<typename Buffer_t, typename T>
inline Buffer_t makeBuffer(std::vector<T> const &data, GLenum usage = GL_DYNAMIC_DRAW)
{
    return makeBuffer<Buffer_t>(data.size() * sizeof(T), data.data(), usage);
}


/// @brief Resize the framebuffer attachment.
/// @param fbo The framebuffer object. Must be initialized.
/// @param texture The texture to resize. May be uninitialized.
/// @param size The new dimensions.
/// @param attachment The attachment type.
/// @param format The texture format.
void attachment(ogl::Framebuffer &fbo, ogl::Texture &texture, glm::uvec2 size, GLenum attachment = GL_COLOR_ATTACHMENT0, GLenum format = GL_RGBA32F);
/// @copydoc attachment
void attachment(ogl::Framebuffer &fbo, ogl::Renderbuffer &rbo, glm::uvec2 size, GLenum attachment = GL_DEPTH_STENCIL_ATTACHMENT, GLenum format = GL_DEPTH24_STENCIL8);

/// @brief Attach a buffer to the vertex array object to the last binding.
/// Stride is computed as count * sizeOfGLType(type) for tightly-packed data.
/// If the buffer's id is 0 it is still pushed, but the attribute is not set up.
/// @param vao The vertex array object.
/// @param buff The buffer to attach (moved into the vao.buffers).
/// @param count The number of primitives.
/// @param type The type of a primitive.
/// @param instanced Sets the attrib divisor to 1 if true.
void pushVertexBuffer(ogl::VAO &vao, ogl::Buffer &&buff, std::size_t count, GLenum type, bool instanced = false);


/// @brief Remove the latest buffer from the vertex array object.
/// @param vao The vertex array object.
void popVertexBuffer(ogl::VAO &vao);

}
