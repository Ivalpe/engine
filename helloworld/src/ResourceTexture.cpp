#include "ResourceTexture.h"
// #include "TextureImporter.h" // Necesitarás crear este archivo también
#include "Log.h"

ResourceTexture::ResourceTexture(uint32_t uid) : Resource(uid, ResourceType::TEXTURE) {
}

ResourceTexture::~ResourceTexture() {
    FreeMemory();
}

void ResourceTexture::LoadInMemory() {
    if (loaded) return;

    // Aquí llamarías a tu TextureImporter::Load(this);
    // Por ahora, como placeholder si no tienes el importer hecho:
    // textureID = TextureImporter::Load(this->libraryPath, &width, &height);

    // EJEMPLO PSEUDOCÓDIGO:
    // if (TextureImporter::Load(this)) loaded = true;

    LOG("Loading Texture Resource (Logic pending TextureImporter implementation)");
}

void ResourceTexture::FreeMemory() {
    if (!loaded) return;

    if (textureID) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
    loaded = false;
}

void ResourceTexture::Bind(unsigned int unit) {
    if (!loaded) return;

    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, textureID);
}