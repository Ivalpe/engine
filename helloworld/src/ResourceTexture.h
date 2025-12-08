#pragma once
#include "Resource.h"
#include <SDL3/SDL_opengl.h>

class ResourceTexture : public Resource {
public:
    ResourceTexture(uint32_t uid);
    ~ResourceTexture();

    // ID de textura en OpenGL
    unsigned int textureID = 0;

    // Metadata
    int width = 0;
    int height = 0;
    std::string type; // "diffuse", "specular", etc.

    void LoadInMemory() override;
    void FreeMemory() override;

    // Vincula la textura al slot especificado
    void Bind(unsigned int unit = 0);
};