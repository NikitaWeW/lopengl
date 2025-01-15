#pragma once
#include <string>
#include "glad/gl.h"
#include "utils/Resource.hpp"

class Texture : public Resource
{
private:
    unsigned m_RenderID = 0;
    std::string m_FilePath;
    unsigned char *m_Buffer = nullptr;
    int m_Width = 0, m_Height = 0, m_BPP = 0;
public:
    std::string type; 
    Texture() = default;
    Texture(GLsizei width, GLsizei height, GLenum wrap = GL_REPEAT);
    Texture(std::string const &filepath, bool flip = true, bool srgb = false, GLenum wrap = GL_REPEAT);
    ~Texture();

    void bind(unsigned slot = 0) const;
    void unbind(unsigned slot = 0) const;
    static void unbindStatic(unsigned slot = 0);

    inline std::string const &getFilePath() const { return m_FilePath; }
    inline unsigned getRenderID() const {return m_RenderID; }
    inline int getWidth() const { return m_Width; }
    inline int getHeight() const { return m_Height; }
    inline int getBitsPerPixel() const { return m_BPP; }
};

class MultisampleTexture : public Resource
{
private:
    unsigned m_RenderID = 0;
    std::string m_FilePath;
    unsigned char *m_Buffer = nullptr;
    int m_Width = 0, m_Height = 0, m_BPP = 0;
public:
    std::string type; 
    MultisampleTexture() = default;
    MultisampleTexture(GLsizei width, GLsizei height, GLenum wrap = GL_REPEAT);
    ~MultisampleTexture();

    void bind(unsigned slot = 0) const;
    void unbind(unsigned slot = 0) const;

    inline std::string const &getFilePath() const { return m_FilePath; }
    inline unsigned getRenderID() const {return m_RenderID; }
    inline int getWidth() const { return m_Width; }
    inline int getHeight() const { return m_Height; }
    inline int getBitsPerPixel() const { return m_BPP; }
};