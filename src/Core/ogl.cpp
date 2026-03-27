#include "ogl.hpp"
#include "Logging.hpp"
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <string>
#include <unordered_map>

static bool compileProgramShader(ogl::Program::Shader &shader) noexcept {
    shader.id = glCreateShader(shader.type);
    char *source = &*shader.source.begin();
    glShaderSource(shader.id, 1, &source, nullptr);
    glCompileShader(shader.id);
    int success;
    glGetShaderiv(shader.id, GL_COMPILE_STATUS, &success);
    if(!success) {
        GLint log_size;
        glGetShaderiv(shader.id, GL_INFO_LOG_LENGTH, &log_size);
        if(log_size > 0) {
            std::string log;
            log.resize(log_size);
            glGetShaderInfoLog(shader.id, log_size, nullptr, &log[0]);
            LOG_ERROR("Failed to compile \"{}\" shader: \n{}", shader.path, log);
        }
        return false;
    }
    return true;
}

static bool linkProgram(ogl::Program &program) noexcept {
    program.id = glCreateProgram();
    for(auto const &shader : program.shaders) {
        glAttachShader(program.id, shader.id);
    }
    glLinkProgram(program.id);

    int success;
    glGetProgramiv(program.id, GL_LINK_STATUS, &success);
    if(!success) {
        GLint log_size;
        glGetProgramiv(program.id, GL_INFO_LOG_LENGTH, &log_size);
        if(log_size > 0) {
            std::string log;
            log.resize(log_size);
            glGetProgramInfoLog(program.id, log_size, nullptr, &log[0]);
            LOG_ERROR(log);
        }
        return false;
    }
    return true;
}
static std::string getShaderTypeString(GLenum type) noexcept {
    switch (type)
    {
    case GL_VERTEX_SHADER:   return "vertex";
    case GL_GEOMETRY_SHADER: return "geometry";
    case GL_FRAGMENT_SHADER: return "fragment";
    case GL_COMPUTE_SHADER:  return "compute";
    default:                 return "unknown type";
    }
}
static std::string getFramebufferStatusString(GLenum status) {
    switch (status) {
    case GL_FRAMEBUFFER_COMPLETE:                       return "GL_FRAMEBUFFER_COMPLETE";
    case GL_FRAMEBUFFER_UNDEFINED:                      return "GL_FRAMEBUFFER_UNDEFINED";
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:          return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:  return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:         return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:         return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
    case GL_FRAMEBUFFER_UNSUPPORTED:                    return "GL_FRAMEBUFFER_UNSUPPORTED";
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:         return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:       return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
    default:                                            return "Unknown Framebuffer Status";
    }
}

/// @brief Process the include directives in the file.
/// @return Processed string of the file. IMPORTANT: is not null-terminated.
static std::string loadFileWithIncludes(std::string path, std::string_view includeIdentifier = "#include", std::string prevDir = "")
{
	auto terminatedInclude = std::string{includeIdentifier} + ' ';

	std::ifstream file;

    std::string fullPath = "";
    if(std::filesystem::exists(prevDir + path)) // Relative
        fullPath = prevDir + path;
    else if(std::filesystem::exists(path)) // Absolute
        fullPath = path;
    else {
		LOG_ERROR("Could not open the file \"{}\"!", path);
		return "";
	}

    if(fullPath != "")
    {
        file = std::ifstream(fullPath);
        prevDir = fullPath.substr(0, std::min(fullPath.find_last_of("/\\") + 1, fullPath.size()));
    }


    assert(file.is_open());

	std::string fullSourceCode = "", lineBuffer;
	while(std::getline(file, lineBuffer))
	{
		if(lineBuffer.find(terminatedInclude) == 0)
		{
			// Remove the include identifier, this will cause the path to remain
			lineBuffer.erase(0, terminatedInclude.size());

			// Remove quotation marks from the include-string, in case there are any
			auto lineBufferQuotationMarkPositions = std::remove(lineBuffer.begin(), lineBuffer.end(), '\"');
			while(lineBuffer.cend() != lineBufferQuotationMarkPositions) {
				lineBuffer.erase(lineBufferQuotationMarkPositions, lineBuffer.cend());
				lineBufferQuotationMarkPositions = std::remove(lineBuffer.begin(), lineBuffer.end(), '\"');
			}

			fullSourceCode += loadFileWithIncludes(lineBuffer, includeIdentifier, prevDir);

			continue;
		} else {
            fullSourceCode += lineBuffer + '\n';
        }
	}

	file.close();

	return fullSourceCode;
}

const std::unordered_map<std::string, GLenum> gExtensionToShaderType = {
    {".vert", GL_VERTEX_SHADER},
    {".vsh",  GL_VERTEX_SHADER},
    {".vs",   GL_VERTEX_SHADER},
    {".geom", GL_GEOMETRY_SHADER},
    {".gsh",  GL_GEOMETRY_SHADER},
    {".gs",   GL_GEOMETRY_SHADER},
    {".frag", GL_FRAGMENT_SHADER},
    {".fsh",  GL_FRAGMENT_SHADER},
    {".fs",   GL_FRAGMENT_SHADER},
    {".comp", GL_COMPUTE_SHADER},
    {".csh",  GL_COMPUTE_SHADER},
    {".cs",   GL_COMPUTE_SHADER},
};

static ogl::Program collectShaders(std::string_view dirpath)
{
    ogl::Program program;
    program.dirpath = dirpath;
    if(!std::filesystem::exists(program.dirpath))
    {
        LOG_ERROR("Failed to collect shaders from a non-existing directory \"{}\"", program.dirpath);
        return program;
    }
    for(auto const &directoryEntry : std::filesystem::recursive_directory_iterator{dirpath}) {
        if(!std::filesystem::is_regular_file(directoryEntry.path())) continue; 
        ogl::Program::Shader shader;

        std::string extension = directoryEntry.path().string().substr(directoryEntry.path().string().find_last_of('.'), directoryEntry.path().string().size());
        if(gExtensionToShaderType.find(extension) == gExtensionToShaderType.end()) continue;
        shader.type = gExtensionToShaderType.at(extension);
        shader.path = directoryEntry.path().string();
        
        // std::ifstream filestream{directoryEntry.path()};
        // shader.source = std::string{std::istreambuf_iterator<char>{filestream}, std::istreambuf_iterator<char>{}};
        shader.source = loadFileWithIncludes(directoryEntry.path().string(), "#include");
        shader.source.push_back('\0');
        program.shaders.emplace_back(std::move(shader));
    }

    return program;
}

std::size_t ogl::getSizeOfGLType(GLenum type)
{
    switch (type) {
    case GL_BYTE:              return sizeof(GLbyte);
    case GL_UNSIGNED_BYTE:     return sizeof(GLubyte);
    case GL_SHORT:             return sizeof(GLshort);
    case GL_UNSIGNED_SHORT:    return sizeof(GLushort);
    case GL_INT:               return sizeof(GLint);
    case GL_UNSIGNED_INT:      return sizeof(GLuint);
    case GL_FLOAT:             return sizeof(GLfloat);
    case GL_DOUBLE:            return sizeof(GLdouble);
    case GL_BOOL:              return sizeof(GLboolean);

    case GL_HALF_FLOAT:        return 2;
    case GL_FIXED:             return 4;

    case GL_FLOAT_VEC2:        return sizeof(GLfloat) * 2;
    case GL_FLOAT_VEC3:        return sizeof(GLfloat) * 3;
    case GL_FLOAT_VEC4:        return sizeof(GLfloat) * 4;
    case GL_INT_VEC2:          return sizeof(GLint) * 2;
    case GL_INT_VEC3:          return sizeof(GLint) * 3;
    case GL_INT_VEC4:          return sizeof(GLint) * 4;
    case GL_UNSIGNED_INT_VEC2: return sizeof(GLuint) * 2;
    case GL_UNSIGNED_INT_VEC3: return sizeof(GLuint) * 3;
    case GL_UNSIGNED_INT_VEC4: return sizeof(GLuint) * 4;
    case GL_BOOL_VEC2:         return sizeof(GLboolean) * 2;
    case GL_BOOL_VEC3:         return sizeof(GLboolean) * 3;
    case GL_BOOL_VEC4:         return sizeof(GLboolean) * 4;

    case GL_FLOAT_MAT2:        return sizeof(GLfloat) * 2 * 2;
    case GL_FLOAT_MAT3:        return sizeof(GLfloat) * 3 * 3;
    case GL_FLOAT_MAT4:        return sizeof(GLfloat) * 4 * 4;
    case GL_FLOAT_MAT2x3:      return sizeof(GLfloat) * 2 * 3;
    case GL_FLOAT_MAT2x4:      return sizeof(GLfloat) * 2 * 4;
    case GL_FLOAT_MAT3x2:      return sizeof(GLfloat) * 3 * 2;
    case GL_FLOAT_MAT3x4:      return sizeof(GLfloat) * 3 * 4;
    case GL_FLOAT_MAT4x2:      return sizeof(GLfloat) * 4 * 2;
    case GL_FLOAT_MAT4x3:      return sizeof(GLfloat) * 4 * 3;

    case GL_DOUBLE_MAT2:       return sizeof(GLdouble) * 2 * 2;
    case GL_DOUBLE_MAT3:       return sizeof(GLdouble) * 3 * 3;
    case GL_DOUBLE_MAT4:       return sizeof(GLdouble) * 4 * 4;
    case GL_DOUBLE_MAT2x3:     return sizeof(GLdouble) * 2 * 3;
    case GL_DOUBLE_MAT2x4:     return sizeof(GLdouble) * 2 * 4;
    case GL_DOUBLE_MAT3x2:     return sizeof(GLdouble) * 3 * 2;
    case GL_DOUBLE_MAT3x4:     return sizeof(GLdouble) * 3 * 4;
    case GL_DOUBLE_MAT4x2:     return sizeof(GLdouble) * 4 * 2;
    case GL_DOUBLE_MAT4x3:     return sizeof(GLdouble) * 4 * 3;

    case GL_UNSIGNED_INT_2_10_10_10_REV:  return 4;
    case GL_INT_2_10_10_10_REV:           return 4;
    case GL_UNSIGNED_INT_10F_11F_11F_REV: return 4;

    case GL_SAMPLER_2D:
    case GL_SAMPLER_CUBE:
    case GL_IMAGE_2D:
    case GL_ATOMIC_COUNTER_BUFFER:
        assert(false && "opaque/sampler type has no raw byte size");
        return 0;
    default: 
        assert(false && "unknown opengl type");
        return 0;
    }
}
ogl::Program ogl::compileShader(std::string_view dirpath)
{
    Program program = collectShaders(dirpath);
    if(program.shaders.empty())
        return program; // error message already sent in collectShaders.

    for(Program::Shader &shader : program.shaders) {
        if(!compileProgramShader(shader)) {
            LOG_ERROR("failed to compile {} shader from \"{}\"!", getShaderTypeString(shader.type), dirpath);
            return Program{};
        }
    }

    if(!linkProgram(program)) {
        LOG_ERROR("failed to link program from \"{}\"!", dirpath);
        return Program{};
    }

    return program;
}

bool ogl::recompileShader(Program &program, std::string_view dirpath)
{
    Program newProgram = compileShader(dirpath);
    if(newProgram.id)
    {
        if(program.id)
        {
            for(auto &shader : program.shaders)
                glDeleteShader(shader.id);
            program.shaders.clear();
            glDeleteProgram(program.id);
            program.id = 0;
        }
        program = newProgram;
        return true;
    }

    LOG_WARN("Failed to compile shaders from \"{}\", rolling back!", dirpath);
    return false;
}

int ogl::getUniform(Program const &program, std::string_view name)
{
    if(program.locationCache.find(std::string(name)) != program.locationCache.end()) return program.locationCache[std::string(name)];
    int location = glGetUniformLocation(program.id, name.data());
    program.locationCache[std::string(name)] = location;
    if(location == -1) {
        LOG_WARN("Uniform \"{}\" is not used or does not exist in shaders from \"{}\".", name, program.dirpath);
    }
    return location;
}

int ogl::getUniformBlock(Program const &program, std::string_view name)
{
    if(program.locationCache.find(std::string(name)) != program.locationCache.end()) return program.locationCache[std::string(name)];
    int location = glGetUniformBlockIndex(program.id, name.data());
    program.locationCache[std::string(name)] = location;
    if(location == -1) {
        LOG_WARN("Uniform \"{}\" is not used or does not exist in shaders from \"{}\".", name, program.dirpath);
    }
    return location;
}

int ogl::getStorageBlock(Program const &program, std::string_view name)
{
    if(program.locationCache.find(std::string(name)) != program.locationCache.end()) return program.locationCache[std::string(name)];
    int location = glGetProgramResourceIndex(program.id, GL_SHADER_STORAGE_BLOCK, name.data());
    program.locationCache[std::string(name)] = location;
    if(location == -1) {
        LOG_WARN("Uniform \"{}\" is not used or does not exist in shaders from \"{}\".", name, program.dirpath);
    }
    return location;
}

bool ogl::isComplete(Framebuffer const &fbo)
{
    unsigned status = glCheckNamedFramebufferStatus(fbo.id, GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        LOG_TRACE("non complete framebuffer checked: {}", getFramebufferStatusString(status));
    return status == GL_FRAMEBUFFER_COMPLETE;
}

ogl::Texture ogl::makeTexture(Bitmap<float> const &data, bool srgb)
{
    Texture texture;
    texture.width = data.getWidth();
    texture.height = data.getHeight();

    glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);

    // texture.handle = glGetTextureHandleARB(texture.id);
    // glMakeTextureHandleResidentARB(texture.handle);

    GLenum internalFormat = 0;
    GLenum format = 0;
    
    if(data.getNumComponents() == 3)
    {
        if(srgb)
            internalFormat = GL_SRGB;
        else
            internalFormat = GL_RGB16F;

        format = GL_RGB;
    }
    else if(data.getNumComponents() == 4)
    {
        if(srgb)
            internalFormat = GL_SRGB_ALPHA;
        else
            internalFormat = GL_RGBA16F;

        format = GL_RGBA;
    }
    else
    {
        assert(false && "invalid number of texture channels");
    }

    bool small = data.getWidth() * data.getHeight() < 10000;

    glTextureStorage2D(texture.id, small ? 1 : 2, internalFormat, data.getWidth(), data.getHeight());
    glTextureSubImage2D(texture.id, 0, 0, 0, data.getWidth(), data.getHeight(), format, GL_FLOAT, data.getData());

    if(small) {
        glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
        glGenerateTextureMipmap(texture.id);
        glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glTextureParameteri(texture.id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texture.id, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return texture;
}

ogl::Cubemap ogl::makeCubemap(std::array<Bitmap<float>, 6> const &data)
{
    ogl::Cubemap cubemap;
    cubemap.width = data[0].getWidth();
    cubemap.height = data[0].getHeight();
    cubemap.numSamples = 1;

    GLenum internalFormat = data[0].getNumComponents() == 4 ? GL_RGBA32F : GL_RGB32F;
    GLenum format = data[0].getNumComponents() == 4 ? GL_RGBA : GL_RGB;

    assert(cubemap.width == cubemap.height);
    unsigned faceSize = cubemap.width;
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &cubemap.id);
    glTextureStorage2D(
        cubemap.id,
        1,
        internalFormat,
        faceSize,
        faceSize
    );

    for(int i = 0; i < 6; ++i){
        glTextureSubImage3D(
            cubemap.id, 
            0,       // layer
            0, 0, i, // x,y,z
            cubemap.width, cubemap.height, // 2D image dimensions
            1,       // depth
            format,  // format
            GL_FLOAT,// data type
            data[i].getData()
        );
        
    }

    return cubemap;
}

void ogl::attachment(ogl::Framebuffer &fbo, ogl::Texture &texture, glm::uvec2 size, GLenum attachment, GLenum format)
{
    if(texture.id != 0)
    {
        glDeleteTextures(1, &texture.id);
        texture.id = 0;
    }
    glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);

    if(texture.numSamples == 1)
        glTextureStorage2D(texture.id, 1, format, size.x, size.y);
    else
        glTextureStorage2DMultisample(texture.id, texture.numSamples, format, size.x, size.y, true);

    assert(fbo.id != 0 && "invalid fbo");
    glNamedFramebufferTexture(fbo.id, attachment, texture.id, 0);
    assert(ogl::isComplete(fbo) && "invalid fbo");
}
void ogl::attachment(ogl::Framebuffer &fbo, ogl::Renderbuffer &rbo, glm::uvec2 size, GLenum attachment, GLenum format)
{
    if(rbo.id != 0)
    {
        glDeleteRenderbuffers(1, &rbo.id);
        rbo.id = 0;
    }
    glCreateRenderbuffers(1, &rbo.id);

    if(rbo.numSamples == 1)
        glNamedRenderbufferStorage(rbo.id, GL_DEPTH24_STENCIL8, size.x, size.y);
    else
        glNamedRenderbufferStorageMultisample(rbo.id, rbo.numSamples, GL_DEPTH24_STENCIL8, size.x, size.y);

    assert(fbo.id != 0 && "invalid fbo");
    glNamedFramebufferRenderbuffer(fbo.id, attachment, GL_RENDERBUFFER, rbo.id);
    assert(ogl::isComplete(fbo) && "invalid fbo");
}
void ogl::pushVertexBuffer(ogl::VAO &vao, ogl::Buffer &&buff, std::size_t count, GLenum type, bool instanced)
{
    if(buff.id == 0) 
    {
        vao.buffers.emplace_back(std::move(buff));
        return;
    }
    unsigned attrib = vao.buffers.size();
    unsigned binding = vao.buffers.size();
    glVertexArrayVertexBuffer(vao.id, binding, buff.id, 0, count * ogl::getSizeOfGLType(type));
    glEnableVertexArrayAttrib(vao.id, attrib);
    switch(type)
    {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
        glVertexArrayAttribIFormat(vao.id, attrib, count, type, 0);
        break;
    default:
        glVertexArrayAttribFormat(vao.id, attrib, count, type, GL_FALSE, 0);
        break;
    }
    glVertexArrayAttribBinding(vao.id, attrib, binding);
    if(instanced)
        glVertexArrayBindingDivisor(vao.id, binding, 1);
    vao.buffers.emplace_back(std::move(buff));
}

void ogl::popVertexBuffer(ogl::VAO &vao)
{
    vao.buffers.pop_back();
    unsigned attrib = vao.buffers.size();
    unsigned binding = vao.buffers.size();
    glDisableVertexArrayAttrib(vao.id, attrib);
    glVertexArrayVertexBuffer(vao.id, binding, 0, 0, 0);
}
