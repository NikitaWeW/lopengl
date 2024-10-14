#pragma once
#include <string>

bool compileShader(unsigned &shader, const char *shaderSource, const int mode,  std::string &log);
bool linkProgram(unsigned &program, unsigned vertexShaderID, unsigned fragmentShaderID, std::string &log);

struct Shader {
public:
    std::string VertexShaderSource;
    std::string FragmentShaderSource;
    unsigned VertexShaderID = 0;
    unsigned FragmentShaderID = 0;
    unsigned ShaderProgramID = 0;
    Shader();
    ~Shader();
    void bind() const;
    void unbind() const;
    unsigned getUniform(const char *name) const;
    bool ParceShaderFile(std::string const &filepath);
    bool CompileShaders();
};
