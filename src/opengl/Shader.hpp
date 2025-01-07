#pragma once
#include <string>
#include <vector>
#include <map>
#include "utils/Resource.hpp"

class ShaderProgram : public Resource {
public:
    struct Shader {
        unsigned renderID;
        unsigned type;
        std::string source;
    };
private:
    mutable std::map<std::string, int> m_UniformLocationCache;
    std::string m_filepath;
    std::string m_log;
    unsigned m_renderID = 0;

    std::vector<Shader> m_shaders;

    void deallocate();
public:
    ShaderProgram();
    ShaderProgram(std::string const &filepath, bool showLog = false);
    ~ShaderProgram();

    void bind() const;
    void unbind() const;

    int getUniform(std::string const &name) const;
    int getUniformBlock(std::string const &name) const;
    bool ParceShaderFile(std::string const &filepath);
    bool CompileShaders();
    
    inline std::string const &getFilePath() const { return m_filepath; }
    inline std::string const &getLog()      const { return m_log; }
    inline unsigned    const &getRenderID() const { return m_renderID; }
};
