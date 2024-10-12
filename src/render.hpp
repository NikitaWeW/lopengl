#pragma once
#include <string>
#include <vector>
#include <glad/gl.h>
#include <logger.h>

void GLErrorCallback(GLenum error);
#define GLCALL(x) \
    do { \
        x; \
        while (GLenum GLerror = glGetError()) { \
            char const *errorString;\
            switch (GLerror) {\
                case GL_NO_ERROR:\
                    errorString = "No error";\
                    break;\
                case GL_INVALID_ENUM:\
                    errorString = "Invalid enum";\
                    break;\
                case GL_INVALID_VALUE:\
                    errorString = "Invalid value";\
                    break;\
                case GL_INVALID_OPERATION:\
                    errorString = "Invalid operation";\
                    break;\
                case GL_STACK_OVERFLOW:\
                    errorString = "Stack overflow";\
                    break;\
                case GL_STACK_UNDERFLOW:\
                    errorString = "Stack underflow";\
                    break;\
                case GL_OUT_OF_MEMORY:\
                    errorString = "Out of memory";\
                    break;\
                case GL_INVALID_FRAMEBUFFER_OPERATION:\
                    errorString = "Invalid framebuffer operation";\
                    break;\
                default:\
                    errorString = "Unknown error";\
                    break;\
                }\
            LOG_ERROR("opengl error: %s", errorString); \
        } \
    } while (0)

bool compileShader(unsigned &shader, const char *shaderSource, const int mode,  std::string &log);
bool linkProgram(unsigned &program, unsigned vertexShaderID, unsigned fragmentShaderID, std::string &log);

struct ShaderProgram {
public:
    std::string VertexShaderSource;
    std::string FragmentShaderSource;
    unsigned VertexShaderID = 0;
    unsigned FragmentShaderID = 0;
    unsigned ShaderProgramID = 0;
    ShaderProgram();
    ~ShaderProgram();
    void ParceShaderFile(std::string const &filepath);
    bool CompileShaders();
};


class VertexBuffer {
private:
    unsigned m_RenderID;
public:
    VertexBuffer(const void *data, size_t size);
    ~VertexBuffer();
    void bind() const;
    void unbind() const;
};

class IndexBuffer {
private:
    unsigned m_RenderID;
public:
    IndexBuffer(const GLuint *data, size_t count);
    ~IndexBuffer();
    void bind() const;
    void unbind() const;
};

struct VertexBufferlayoutElement {
    unsigned type;
    unsigned count;
    bool normalised;
};

class VertexBufferlayout {
private:
    std::vector<VertexBufferlayoutElement> m_elements;
    unsigned m_stride;
public:
    VertexBufferlayout();
    ~VertexBufferlayout();
    template <typename T>
    void push(unsigned const count);
    inline unsigned getStride() const;
    inline std::vector<VertexBufferlayoutElement> const &getElements() const;
};
class VertexArray {
private:
    unsigned m_RenderID;
public:
    VertexArray();
    ~VertexArray();
    void bind() const;
    void unbind() const;
    void addBuffer(VertexBuffer const &VB, VertexBufferlayout const &layout);
};
