#include "glad/gl.h"
#include <string>
#include <fstream>
#include <sstream>
#include <array>

#include "Shader.hpp"
#include "GlCall.h"

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
Shader::Shader() = default;
Shader::~Shader() {
    if(VertexShaderID) GLCALL(glDeleteShader(VertexShaderID));
    if(FragmentShaderID) GLCALL(glDeleteShader(FragmentShaderID));
    if(ShaderProgramID) GLCALL(glDeleteProgram(ShaderProgramID));
}
void Shader::bind() const {
    GLCALL(glUseProgram(ShaderProgramID));
}
void Shader::unbind() const {
    GLCALL(glUseProgram(0));
}
int Shader::getUniform(std::string const &name)
{
    if(m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) return m_UniformLocationCache[name];
    GLCALL(int location = glGetUniformLocation(ShaderProgramID, name.c_str()));
    m_UniformLocationCache[name] = location;
    return location;
}
bool Shader::ParceShaderFile(std::string const &filepath)
{
    std::ifstream fileStream(filepath, std::ios::in);
    if(!fileStream) {
        LOG_ERROR("failed to open %s", filepath);
        return false;
    }
    std::array<std::stringstream, 2> shaderSourceStreams;
    unsigned currentIndex;
    std::string line;
    while (getline(fileStream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                currentIndex = 0;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                currentIndex = 1;
            }
        }
        else
        {
            shaderSourceStreams[currentIndex] << line << '\n';
        }
    }
    VertexShaderSource = shaderSourceStreams[0].str();
    FragmentShaderSource = shaderSourceStreams[1].str();
    return true;
}
bool Shader::CompileShaders() {
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