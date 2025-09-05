#include "main.hpp"

static void resizeColorAttachment(ogl::Framebuffer &fbo, ogl::TextureMS &texture, glm::ivec2 size, GLenum attachment = GL_COLOR_ATTACHMENT0)
{
    glDeleteTextures(1, &texture.getRenderID());
    texture.getRenderID() = 0;
    glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &texture.getRenderID());
    glTextureStorage2DMultisample(texture.getRenderID(), NUM_SAMPLES, GL_RGBA32F, size.x, size.y, true);
    if(fbo.getRenderID() != 0)
    {
        fbo.attach(texture, GL_COLOR_ATTACHMENT0);
        assert(fbo.isComplete());
    }
}
static void resizeColorAttachment(ogl::Framebuffer &fbo, ogl::Texture &texture, glm::ivec2 size, GLenum attachment = GL_COLOR_ATTACHMENT0)
{
    glDeleteTextures(1, &texture.getRenderID());
    texture.getRenderID() = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture.getRenderID());
    glTextureStorage2D(texture.getRenderID(), 1, GL_RGBA32F, size.x, size.y);
    if(fbo.getRenderID() != 0)
    {
        fbo.attach(texture, GL_COLOR_ATTACHMENT0);
        assert(fbo.isComplete());
    }
}

void drawFrame(Data &data)
{
    glm::ivec2 prevDim = data.windowSize;
    glfwGetFramebufferSize(data.window, &data.windowSize.x, &data.windowSize.y);
    if(data.windowSize != prevDim)
    { // resize drawbuffers
        resizeColorAttachment(data.mainFBO, data.mainColor, data.windowSize);
        glNamedRenderbufferStorageMultisample(data.mainRBO.getRenderID(), NUM_SAMPLES, GL_DEPTH24_STENCIL8, data.windowSize.x, data.windowSize.y);

        resizeColorAttachment(data.displayFBO, data.displayTexture, data.windowSize);
        glNamedRenderbufferStorage(data.displayRBO.getRenderID(), GL_DEPTH24_STENCIL8, data.windowSize.x, data.windowSize.y);
    }
    if(data.mainFBO.getRenderID() == 0)
    {
        data.mainFBO = ogl::Framebuffer{0}; // dummy argument
        data.mainFBO.attach(data.mainColor, GL_COLOR_ATTACHMENT0);
        data.mainFBO.attach(data.mainRBO, GL_DEPTH_STENCIL_ATTACHMENT);
        assert(data.mainFBO.isComplete());
    }
    if(data.displayFBO.getRenderID() == 0)
    {
        data.displayFBO = ogl::Framebuffer{0};
        data.displayFBO.attach(data.displayTexture, GL_COLOR_ATTACHMENT0);
        data.displayFBO.attach(data.displayRBO, GL_DEPTH_STENCIL_ATTACHMENT);
        assert(data.displayFBO.isComplete());
    }
    processInput(data);
    glViewport(0, 0, data.windowSize.x, data.windowSize.y);

    // ==========================

    data.mainFBO.bind();

    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);

    // ==============
    // draw a skybox 
    // ==============

    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);

    data.skyboxDrawShader.bind();
    data.skybox.bind(0);

    glUniformMatrix4fv(data.skyboxDrawShader.getUniform("u_viewMat"),        1, GL_FALSE, glm::value_ptr(data.viewMat));
    glUniformMatrix4fv(data.skyboxDrawShader.getUniform("u_projectionMat"),  1, GL_FALSE, glm::value_ptr(data.projMat));

    // vertices hard-coded in the shader
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);

    // ============
    // draw a grid 
    // ============

    glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    data.gridShader.bind();

    glUniformMatrix4fv(data.gridShader.getUniform("u_viewMat"),        1, GL_FALSE, glm::value_ptr(data.viewMat));
    glUniformMatrix4fv(data.gridShader.getUniform("u_projectionMat"),  1, GL_FALSE, glm::value_ptr(data.projMat));

    // vertices hard-coded in the shader
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // ============
    // draw a cube 
    // ============

    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);

    data.cubeDrawShader.bind();
    data.texture.bind(0);
    
    glUniformMatrix4fv(data.cubeDrawShader.getUniform("u_modelMat"),        1, GL_FALSE, glm::value_ptr(glm::mat4{1.0f}));
    glUniformMatrix4fv(data.cubeDrawShader.getUniform("u_viewMat"),         1, GL_FALSE, glm::value_ptr(data.viewMat));
    glUniformMatrix4fv(data.cubeDrawShader.getUniform("u_projectionMat"),   1, GL_FALSE, glm::value_ptr(data.projMat));
    
    data.cube.vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, data.cube.count);

    // ============================================
    // draw to a display texture + post processing 
    // ============================================

    glDepthFunc(GL_ALWAYS);

    data.displayFBO.bind();
    data.displayShader.bind();
    data.mainColor.bind(0);
    // vertices hard-coded in the shader
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
    
    // ==========================
    // display a display texture  
    // ==========================

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDepthFunc(GL_ALWAYS);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glBlitNamedFramebuffer(data.displayFBO.getRenderID(), 0, 0, 0, data.windowSize.x, data.windowSize.y, 0, 0, data.windowSize.x, data.windowSize.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void generateTexture(Data &data)
{
    auto start = std::chrono::high_resolution_clock::now();

    data.texture = ogl::Cubemap{0};
    glTextureStorage2D(
        data.texture.getRenderID(),
        1,
        TEXTURE_FORMAT,
        data.inputs.textureSize,
        data.inputs.textureSize
    );

    data.cubeGenerateShader.bind();
    glUniform1ui(data.cubeGenerateShader.getUniform("u_seed"), data.inputs.seed);
    glUniform2f( data.cubeGenerateShader.getUniform("u_dimensions"), data.inputs.textureSize, data.inputs.textureSize);
    glViewport(0, 0, data.inputs.textureSize, data.inputs.textureSize);

    data.textureFBO.bind();
    for(unsigned face = 0; face < 6; ++face)
    {
        glUniform1ui(data.cubeGenerateShader.getUniform("u_face"), face);
        glNamedFramebufferTextureLayer(data.textureFBO.getRenderID(), GL_COLOR_ATTACHMENT0, data.texture.getRenderID(), 0, face);
        assert(data.textureFBO.isComplete());
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    data.textureFBO.attach(ogl::Cubemap{}, GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    auto time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 1.0E-3;
    std::cout << "finished generating texture in " << time << "ms.\n";
}
