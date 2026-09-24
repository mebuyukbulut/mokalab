#include "AssetSystem/TextureLoader.h"
#include "Builtin.h"
#include "EngineContext.h"

#include <glad/gl.h>
#include <stb_image.h>


std::shared_ptr<Asset> TextureLoader::load(std::filesystem::path path, IAssetSettings* settings)
{
    std::shared_ptr<Texture> texture = std::make_shared<Texture>(); 

    // RETURN VALUE leri hallet

    texture->_path = path.string();
    texture->_type = GL_TEXTURE_2D;

    std::string pathStr = path.string();

    // Sanal yolla model yükleme
    for(std::string key : Builtin::Texture::All){
        if(key == pathStr){
            // Generated textures
            loadInternal(texture, pathStr);
            texture->_loadStatus = AssetLoadStatus::Complete;
            return texture;
        }
    }

    for(std::string key : Builtin::Icon::All){
        if(key == pathStr){
            TextureSettings* ts = static_cast<TextureSettings*>(settings);
            ts->realPath;
            texture->_loadStatus = AssetLoadStatus::LoadingToCPU;
            texture->_data = stbi_load(ts->realPath.c_str(), &texture->_width, &texture->_height, &texture->_nrChannels, 0);
            //LOG_INFO("{} is ready to upload.", _path.c_str());
            texture->_loadStatus = AssetLoadStatus::ReadyToUpload;
            return texture;            
        }
    }


    //_type = GL_TEXTURE_CUBE_MAP;

    texture->_loadStatus = AssetLoadStatus::LoadingToCPU;
    texture->_data = stbi_load(path.string().c_str(), &texture->_width, &texture->_height, &texture->_nrChannels, 0);
    //LOG_INFO("{} is ready to upload.", _path.c_str());
    texture->_loadStatus = AssetLoadStatus::ReadyToUpload;

    return texture; 
}


void TextureLoader::loadInternal(std::shared_ptr<Texture> texture, std::string path)
{
    if(path == Builtin::Texture::SolidBlack)        createSolidColorTextureRGBA8(texture,   0,   0,   0, 255);
    else if(path == Builtin::Texture::SolidWhite)   createSolidColorTextureRGBA8(texture, 255, 255, 255, 255);
    else if(path == Builtin::Texture::FlatNormal)   createSolidColorTextureRGBA8(texture, 128, 128, 255, 255);
}

void TextureLoader::createSolidColorTextureRGBA8(std::shared_ptr<Texture> texture, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{   
    texture->_type = GL_TEXTURE_2D;

    glGenTextures(1, &texture->_id);
    glBindTexture(GL_TEXTURE_2D, texture->_id);

    uint8_t pixel[4] = { r, g, b, a };

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        1,
        1,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixel
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //glBindTexture(GL_TEXTURE_2D, 0);
}