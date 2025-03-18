#include "glad/gl.h"
#include <string>
#include <fstream>
#include <sstream>
#include <array>
#include <regex>

#include "Shader.hpp"
#include "logger.h"

void ShaderProgram::deallocate() {
    glDeleteProgram(m_renderID);
    for(Shader const &shader : m_shaders) {
        glDeleteShader(shader.renderID);
    }   
}

bool compileShader(ShaderProgram::Shader &shader, std::string &log) {
    shader.renderID = glCreateShader(shader.type);
    char *source = &*shader.source.begin();
    glShaderSource(shader.renderID, 1, &source, nullptr);
    glCompileShader(shader.renderID);
    int success;
    glGetShaderiv(shader.renderID, GL_COMPILE_STATUS, &success);
    if(!success) {
        GLint log_size;
        glGetShaderiv(shader.renderID, GL_INFO_LOG_LENGTH, &log_size);
        if(log_size > 0) {
            log.resize(log_size);
            glGetShaderInfoLog(shader.renderID, log_size, nullptr, &log[0]);

            static std::regex regex{R"(\d+:\d+\(\d+\)[^=])"};
            auto iterator = std::sregex_iterator{log.begin(), log.end(), regex};
            static auto endIterator = std::sregex_iterator{};
            for(; iterator != endIterator; ++iterator) {
                log.insert(log.find(iterator->str()) + iterator->str().size() - 1, "=" + std::to_string(std::stoi(iterator->str().substr(iterator->str().find_first_of(':') + 1, iterator->str().find_first_of('(') - 1)) + shader.fileLine)); 
            }
        }
        return false;
    }
    return true;
}

bool linkProgram(unsigned &program, std::vector<ShaderProgram::Shader> shaders, std::string &log) {
    program = glCreateProgram();
    for(auto const &shader : shaders) {
        glAttachShader(program, shader.renderID);
    }
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

std::string shaderTypeToString(unsigned type) {
    switch (type)
    {
    case GL_VERTEX_SHADER:   return "vertex";
    case GL_GEOMETRY_SHADER: return "geometry";
    case GL_FRAGMENT_SHADER: return "fragment";
    case GL_COMPUTE_SHADER:  return "compute";
    default:                 return "unknown type";
    }
}

ShaderProgram::ShaderProgram() = default;
ShaderProgram::ShaderProgram(std::string const &filepath, bool showLog) : m_filepath(filepath)
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
}
ShaderProgram::~ShaderProgram() {
    if(canDeallocate()) deallocate();
}

void ShaderProgram::bind() const {
    glUseProgram(m_renderID);
}
void ShaderProgram::unbind() const {
    glUseProgram(0);
}
int ShaderProgram::getUniform(std::string const &name) const
{
    if(m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) return m_UniformLocationCache[name];
    int location = glGetUniformLocation(m_renderID, name.c_str());
    m_UniformLocationCache[name] = location;
    if(location == -1) {
        LOG_WARN("uniform \"%s\" in shader \"%s\" is not used.", name.c_str(), getFilePath().c_str());
    }
    return location;
}
int ShaderProgram::getUniformBlock(std::string const &name) const
{
    bind();
    int location = glGetUniformBlockIndex(m_renderID, name.c_str());
    return location;
}
int ShaderProgram::getStorageBlock(std::string const &name) const
{
    bind();
    int location = glGetProgramResourceIndex(m_renderID, GL_SHADER_STORAGE_BLOCK, name.c_str());
    return location;
}
bool ShaderProgram::ParceShaderFile(std::string const &filepath)
{
    std::ifstream fileStream(filepath, std::ios::in);
    if(!fileStream) {
        m_log = "failed to open " + filepath;
        return false;
    }
    std::array<std::stringstream, 5> shaderSourceStreams;
    std::array<unsigned, 5>          shaderStartLines;
    unsigned currentIndex = 0;
    std::string line;
    unsigned lineNum = 1;
    while (getline(fileStream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                currentIndex = 1;
            } else if (line.find("fragment") != std::string::npos)
            {
                currentIndex = 2;
            } else if(line.find("geometry") != std::string::npos) {
                currentIndex = 3;
            } else if(line.find("compute") != std::string::npos) {
                currentIndex = 4;
            } else {
                LOG_WARN("unrecognised #shader statement:\n\t%s <-- here", line.substr(0, line.size() - 1).c_str());
                currentIndex = 0;
            }
            shaderStartLines[currentIndex] = lineNum;
        } else
        {
            shaderSourceStreams[currentIndex] << line << '\n';
        }
        ++lineNum;
    }
    m_shaders.erase(m_shaders.begin(), m_shaders.end());
    if(shaderSourceStreams[1].str().size() > 0) m_shaders.push_back({0, GL_VERTEX_SHADER,   shaderSourceStreams[1].str(), shaderStartLines[1]});
    if(shaderSourceStreams[2].str().size() > 0) m_shaders.push_back({0, GL_FRAGMENT_SHADER, shaderSourceStreams[2].str(), shaderStartLines[2]});
    if(shaderSourceStreams[3].str().size() > 0) m_shaders.push_back({0, GL_GEOMETRY_SHADER, shaderSourceStreams[3].str(), shaderStartLines[3]});
    if(shaderSourceStreams[4].str().size() > 0) m_shaders.push_back({0, GL_COMPUTE_SHADER,  shaderSourceStreams[4].str(), shaderStartLines[4]});

    return true;
}
bool ShaderProgram::CompileShaders() {
    if(canDeallocate()) deallocate();

    m_UniformLocationCache.erase(m_UniformLocationCache.begin(), m_UniformLocationCache.end());
    m_log = "";
    
    for(Shader &shader : m_shaders) {
        if(!compileShader(shader, m_log)) {
            m_log.insert(0, "failed to compile " + shaderTypeToString(shader.type) + " shader\n");
            return false;
        }
    }

    if(!linkProgram(m_renderID, m_shaders, m_log)) {
        m_log.insert(0, "failed to link shader program\n");
        return false;
    }



    return true;
}
