#include "Shader.hpp"
#include <fstream>
#include <cassert>
#include <filesystem>
#include <iostream>

bool compileShader(ogl::ShaderProgram::Shader &shader, std::string &log) noexcept {
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
        }
        return false;
    }
    return true;
}

bool linkProgram(unsigned &program, std::vector<ogl::ShaderProgram::Shader> shaders, std::string &log) noexcept {
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

void ogl::ShaderProgram::deallocate() noexcept
{
    if(m_renderID) glDeleteProgram(m_renderID);
    for(Shader const &shader : m_shaders) {
        if(shader.renderID) glDeleteShader(shader.renderID);
    }   
}

ogl::ShaderProgram::ShaderProgram(std::string const &directory, bool showLog)
{
    if(!collectShaders(directory)) {
        m_log.insert(0, "failed to collect shaders in directory \"" + directory + "\"\n");
        if(showLog) std::cout << getLog();
        throw std::runtime_error{"failed to init shader program"};
    }
    if(!compileShaders()) {
        m_log.insert(0, "failed to compile shaders in directory \"" + directory + "\"\n");
        if(showLog) std::cout << getLog();
        throw std::runtime_error{"failed to init shader program"};
    }
}

ogl::ShaderProgram::~ShaderProgram()
{
    if(canDeallocate()) {
        deallocate();
    }
}

bool ogl::ShaderProgram::collectShaders(std::string const &directory) noexcept
{
    assert(std::filesystem::exists(directory));
    m_dirPath = directory;
    m_log = "";
    m_shaders.erase(m_shaders.begin(), m_shaders.end());
    m_uniformLocationCache.erase(m_uniformLocationCache.begin(), m_uniformLocationCache.end());
    for(auto const &directoryEntry : std::filesystem::recursive_directory_iterator{directory}) {
        if(!std::filesystem::is_regular_file(directoryEntry.path())) continue; 
        Shader shader;

        std::string extension = directoryEntry.path().string().substr(directoryEntry.path().string().find_last_of('.'), directoryEntry.path().string().size());
        if(extension == ".vert") shader.type = GL_VERTEX_SHADER;
        else if(extension == ".geom") shader.type = GL_GEOMETRY_SHADER;
        else if(extension == ".frag") shader.type = GL_FRAGMENT_SHADER;
        else if(extension == ".comp") shader.type = GL_COMPUTE_SHADER;
        else {
            m_log.append("unrecognised shader extension: \"" + directoryEntry.path().extension().string() + "\"\n");
            // return false;
            continue;
        }

        std::ifstream filestream{directoryEntry.path()};
        shader.source = std::string{std::istreambuf_iterator<char>{filestream}, std::istreambuf_iterator<char>{}};
        m_shaders.emplace_back(std::move(shader));
    }
    return true;
}

std::string shaderTypeToString(unsigned type) noexcept {
    switch (type)
    {
    case GL_VERTEX_SHADER:   return "vertex";
    case GL_GEOMETRY_SHADER: return "geometry";
    case GL_FRAGMENT_SHADER: return "fragment";
    case GL_COMPUTE_SHADER:  return "compute";
    default:                 return "unknown type";
    }
}
bool ogl::ShaderProgram::compileShaders() noexcept
{
    if(canDeallocate()) 
        deallocate();

    m_uniformLocationCache.erase(m_uniformLocationCache.begin(), m_uniformLocationCache.end());
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

int ogl::ShaderProgram::getUniform(std::string const &name) const noexcept
{
    if(m_uniformLocationCache.find(name) != m_uniformLocationCache.end()) return m_uniformLocationCache[name];
    int location = glGetUniformLocation(m_renderID, name.c_str());
    m_uniformLocationCache[name] = location;
    // if(location == -1) {
    //     std::cout << "uniform \"" << name << "\" in shaders \"" << getPath() << "\" is not used or does not exist.\n";
    // }
    return location;
}

int ogl::ShaderProgram::getUniformBlock(std::string const &name) const noexcept
{
    int location = glGetUniformBlockIndex(m_renderID, name.c_str());
    return location;
}

int ogl::ShaderProgram::getStorageBlock(std::string const &name) const noexcept
{
    int location = glGetProgramResourceIndex(m_renderID, GL_SHADER_STORAGE_BLOCK, name.c_str());
    return location;
}

void ogl::ShaderProgram::bind(unsigned slot) const noexcept { glUseProgram(m_renderID); }
