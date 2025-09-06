#include "main.hpp"

static void helpMarker(std::string_view desc)
{
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if(ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc.data());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static void save(Data &data)
{
    int const numComponents = 3;

    std::array<Bitmap<unsigned char>, eqr::NUM_CUBEMAP_FACES> cubemapFaces;

    for(int i = 0; i < eqr::NUM_CUBEMAP_FACES; ++i){
        ogl::Texture faceTexture{GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE};
        glTextureStorage2D(faceTexture.getRenderID(), 1, TEXTURE_FORMAT, data.inputs.textureSize, data.inputs.textureSize);

        glCopyImageSubData(
            data.texture.getRenderID(),     // src name
            GL_TEXTURE_CUBE_MAP,            // src target
            0,                              // src level
            0, 0, i,                        // src x,y,z
            faceTexture.getRenderID(),      // dst name
            GL_TEXTURE_2D,                  // dst target
            0,                              // dst level
            0, 0, 0,                        // dst x,y,z
            data.inputs.textureSize,        // width
            data.inputs.textureSize,        // height
            1                               // depth
        );

        cubemapFaces[i] = Bitmap<unsigned char>{data.inputs.textureSize, data.inputs.textureSize, numComponents};

        // FIXME: pbo maybe?
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glGetTextureImage(faceTexture.getRenderID(), 0, GL_RGB, GL_UNSIGNED_BYTE, data.inputs.textureSize * data.inputs.textureSize * numComponents * sizeof(char), cubemapFaces[i].getData());
    }
    Bitmap<unsigned char> equirectangularImage = eqr::fromCubemap(cubemapFaces);

    stbi_write_png("image.png", equirectangularImage.getWidth(), equirectangularImage.getHeight(), numComponents, equirectangularImage.getData(), equirectangularImage.getWidth() * numComponents);
}

void ui(Data &data)
{
    ImGui::Begin(CONFIG_WINDOW_NAME.data());
    if(ImGui::InputFloat("seed", &data.inputs.seed))
        generateTexture(data);
    if(ImGui::Button("random seed")) 
        randomSeed(data);

    
    ImGui::Separator();
    
    // if(ImGui::InputScalar("size", ImGuiDataType_U32, &data.inputs.textureSize))
    //     generateTexture(data);

    if(ImGui::Button("save"))
        save(data);
    helpMarker("Save into the image.png as an equirectangular image (panorama)");

    ImGui::End();
}