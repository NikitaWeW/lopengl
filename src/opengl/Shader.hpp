#pragma once
#include <string>
#include <map>
// TODO: remake that
bool compileShader(unsigned &shader, const char *shaderSource, const int mode,  std::string &log);
bool linkProgram(unsigned &program, unsigned vertexShaderID, unsigned fragmentShaderID, std::string &log);

struct Shader {
private:
    mutable std::map<std::string, int> m_UniformLocationCache;
    mutable bool m_managing = false;
    std::string m_filepath;
    std::string m_log;
public:
    std::string VertexShaderSource;
    std::string FragmentShaderSource;
    unsigned VertexShaderID = 0;
    unsigned FragmentShaderID = 0;
    unsigned ShaderProgramID = 0;

public:
    Shader();
    Shader(std::string const &filepath, bool showLog = false);
    ~Shader();
    Shader(Shader const &other);
    Shader(Shader &&other);
    void operator=(Shader const &other);
    void copy(Shader const &other);
    void swap(Shader &&other);
    void bind() const;
    void unbind() const;
    int getUniform(std::string const &name) const;
    bool ParceShaderFile(std::string const &filepath);
    bool CompileShaders();
    inline std::string const &getFilePath() const { return m_filepath; }
    inline std::string const &getLog() const { return m_log; }
};
