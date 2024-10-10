#include "render.hpp"
#include <fstream>
#include <utility>
#include <sstream>
#include <glad/gl.h>
#include <logger.h>
#include <array>

bool compileShader(unsigned &shader, const char *shaderSource, const int mode, std::string &log) {
    shader = glCreateShader(mode);
    GLCALL(glShaderSource(shader, 1, &shaderSource, nullptr));
    GLCALL(glCompileShader(shader));
    int success;
    GLCALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    if(!success) {
        GLint log_size;
        GLCALL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size));
        if(log_size > 0) {
            log.resize(log_size);
            GLCALL(glGetShaderInfoLog(shader, log_size, nullptr, &log[0]));
        }
        return false;
    }
    return true;
}
bool linkProgram(unsigned &program, unsigned vertexShaderID, unsigned fragmentShaderID, std::string &log) {
    program = glCreateProgram();
    GLCALL(glAttachShader(program, vertexShaderID));
    GLCALL(glAttachShader(program, fragmentShaderID));
    GLCALL(glLinkProgram(program));

    int success;
    GLCALL(glGetProgramiv(program, GL_LINK_STATUS, &success));
    if(!success) {
        GLint log_size;
        GLCALL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size));
        if(log_size > 0) {
            log.resize(log_size);
            GLCALL(glGetProgramInfoLog(program, log_size, nullptr, &log[0]));
        }
        return false;
    }
    return true;
}

ShaderProgram::ShaderProgram() = default;
ShaderProgram::~ShaderProgram() {
    if(VertexShaderID) GLCALL(glDeleteShader(VertexShaderID));
    if(FragmentShaderID) GLCALL(glDeleteShader(FragmentShaderID));
    if(ShaderProgramID) GLCALL(glDeleteProgram(ShaderProgramID));
}
void ShaderProgram::ParceShaderFile(std::string const &filepath) {
        std::ifstream fileStream(filepath, std::ios::in);
        std::array<std::stringstream, 2> shaderSourceStreams;
        unsigned currentIndex;
        std::string line;
        while(getline(fileStream, line)) {
            if(line.find("#shader") != std::string::npos) {
                if(line.find("vertex") != std::string::npos) {
                    currentIndex = 0;
                } else if(line.find("fragment") != std::string::npos) {
                    currentIndex = 1;
                } 
            } else {
                shaderSourceStreams[currentIndex] << line << '\n';
            }
        }
        VertexShaderSource = shaderSourceStreams[0].str();
        FragmentShaderSource = shaderSourceStreams[1].str();
    }
bool ShaderProgram::CompileShaders() {
    std::string log;
    if(!compileShader(VertexShaderID, VertexShaderSource.c_str(), GL_VERTEX_SHADER, log)) {
        LOG_ERROR("failed to compile vertex shader! log:\n%s", log.c_str());
        return false;
    }
    if(!compileShader(FragmentShaderID, FragmentShaderSource.c_str(), GL_FRAGMENT_SHADER, log)) {
        LOG_ERROR("failed to compile fragment shader! log:\n%s", log.c_str());
        return false;
    }
    if(!linkProgram(ShaderProgramID, VertexShaderID, FragmentShaderID, log)) {
        LOG_ERROR("failed to link shader program! log:\n%s", log.c_str());
        return false;
    }
    return true;
}

VertexBuffer::VertexBuffer(const void *data, size_t size) {
    GLCALL(glGenBuffers(1, &m_RenderID));
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, m_RenderID));
    GLCALL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::~VertexBuffer() {
    GLCALL(glDeleteBuffers(1, &m_RenderID));
}

void VertexBuffer::bind() const {
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, m_RenderID));
}

void VertexBuffer::unbind() const {
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
