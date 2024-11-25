#include "glad/gl.h"
#include <string>
#include <fstream>
#include <sstream>
#include <array>

#include "Shader.hpp"
#include "logger.h"

bool compileShader(unsigned &shader, const char *shaderSource, const int mode, std::string &log) {
    shader = glCreateShader(mode);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        GLint log_size;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
        if(log_size > 0) {
            log.resize(log_size);
            glGetShaderInfoLog(shader, log_size, nullptr, &log[0]);
        }
        return false;
    }
    return true;
}
bool linkProgram(unsigned &program, unsigned vertexShaderID, unsigned fragmentShaderID, std::string &log) {
    program = glCreateProgram();
    glAttachShader(program, vertexShaderID);
    glAttachShader(program, fragmentShaderID);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success) {
        GLint log_size;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
        if(log_size > 0) {
            log.resize(log_size);
            glGetProgramInfoLog(program, log_size, nullptr, &log[0]);
        }
        return false;
    }
    return true;
}
Shader::Shader() = default;
Shader::Shader(std::string const &filepath, bool showLog) : m_filepath(filepath)
{
    if(!ParceShaderFile(filepath)) {
        if(showLog) {
            LOG_ERROR("shader name: %s\n%s", m_filepath.c_str(), m_log.c_str());
        }
        throw std::runtime_error("failed to parce shaders!");
    }
    if(!CompileShaders()) {
        if(showLog) {
            LOG_ERROR("shader name: %s\n%s", m_filepath.c_str(), m_log.c_str());
        }
        throw std::logic_error("failed to compile shaders!");
    }
    m_managing = true;
}
Shader::~Shader() {
    if(m_managing) {
        glDeleteShader(VertexShaderID);
        glDeleteShader(FragmentShaderID);
        glDeleteProgram(ShaderProgramID);
    }
}
Shader::Shader(Shader const &other) {
    copy(other);
}
Shader::Shader(Shader &&other) {
    swap(std::forward<Shader>(other));
}
void Shader::operator=(Shader const &other) {
    copy(other);
}
void Shader::copy(Shader const &other) {
    std::swap(m_managing, other.m_managing);
    VertexShaderID = other.VertexShaderID;
    FragmentShaderID = other.FragmentShaderID;
    ShaderProgramID = other.ShaderProgramID;
}
void Shader::swap(Shader &&other) {
    std::swap(m_managing, other.m_managing);
    std::swap(VertexShaderID, other.VertexShaderID);
    std::swap(FragmentShaderID, other.FragmentShaderID);
    std::swap(ShaderProgramID, other.ShaderProgramID);
}
void Shader::bind() const {
    glUseProgram(ShaderProgramID);
}
void Shader::unbind() const {
    glUseProgram(0);
}
int Shader::getUniform(std::string const &name) const
{
    if(m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) return m_UniformLocationCache[name];
    int location = glGetUniformLocation(ShaderProgramID, name.c_str());
    m_UniformLocationCache[name] = location;
    return location;
}
bool Shader::ParceShaderFile(std::string const &filepath)
{
    std::ifstream fileStream(filepath, std::ios::in);
    if(!fileStream) {
        LOG_ERROR("failed to open %s", filepath);
        m_log = "failed to open " + filepath;
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
    if(m_managing) {
        glDeleteShader(VertexShaderID);
        glDeleteShader(FragmentShaderID);
        glDeleteProgram(ShaderProgramID);
    }
    m_UniformLocationCache.erase(m_UniformLocationCache.begin(), m_UniformLocationCache.end());
    if(!compileShader(VertexShaderID, VertexShaderSource.c_str(), GL_VERTEX_SHADER, m_log)) {
        m_log.insert(0, "failed to compile vertex shader\n");
        return false;
    }
    if(!compileShader(FragmentShaderID, FragmentShaderSource.c_str(), GL_FRAGMENT_SHADER, m_log)) {
        m_log.insert(0, "failed to compile fragment shader\n");
        return false;
    }
    if(!linkProgram(ShaderProgramID, VertexShaderID, FragmentShaderID, m_log)) {
        m_log.insert(0, "failed to link shader program\n");
        return false;
    }
    return true;
}