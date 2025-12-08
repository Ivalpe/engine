#include "ResourceTexture.h"
#include "TextureImporter.h"
#include "Log.h"
#include <SDL3/SDL_opengl.h>

ResourceTexture::ResourceTexture(uint32_t uid) : Resource(uid, ResourceType::TEXTURE) {
}

ResourceTexture::~ResourceTexture() {
    FreeMemory();
}

void ResourceTexture::LoadInMemory() {
    if (loaded) return;

    if (TextureImporter::Load(this)) {
        loaded = true;
    }
    else {
        LOG("Failed to load texture resource UID: %d", UID);
    }
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
    if (!loaded) LoadInMemory(); // Auto-load si es necesario

    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, textureID);
}