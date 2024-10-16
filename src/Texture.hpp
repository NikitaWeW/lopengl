#include <string>

class Texture
{
private:
    unsigned m_RenderID;
    std::string m_FilePath;
    unsigned char *m_Buffer;
    int m_Width, m_Height, m_BPP;

public:
    Texture(std::string const &filepath);
    ~Texture();

    void bind(unsigned slot = 0) const;
    void unbind() const;

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