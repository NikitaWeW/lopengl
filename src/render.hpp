#pragma once
#include <string>
#include <glad/gl.h>
#include <logger.h>

void GLErrorCallback(GLenum error);
#define GLCALL(x) \
    do { \
        x; \
        while (GLenum GLerror = glGetError()) { \
            GLErrorCallback(GLerror); \
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