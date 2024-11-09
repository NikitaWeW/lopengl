#pragma once
#include <string>

class Texture
{
private:
    unsigned m_RenderID = 0;
    std::string m_FilePath;
    unsigned char *m_Buffer;
    int m_Width, m_Height, m_BPP;
    mutable bool m_managing = false;
public:
    std::string type; 
    Texture(std::string const &filepath, bool flip = true);
    ~Texture();
    Texture(Texture const &other);
    Texture(Texture &&other);
    void operator=(Texture const &other);
    void copy(Texture const &other);
    void swap(Texture &&other);

    void bind(unsigned slot = 0) const;
    void unbind() const;

    inline std::string getFilePath() const {
        return m_FilePath;
    }
    inline int getWidth() const {
        return m_Width;
    }
    inline int getHeight() const {
        return m_Height;
    }
    inline int getBitsPerPixel() const {
        return m_BPP;
    }
};