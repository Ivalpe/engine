#pragma once
#include "Resource.h"
#include <SDL3/SDL_opengl.h> 

class ResourceMesh : public Resource {
public:
    ResourceMesh(uint32_t uid);
    ~ResourceMesh();

    // Datos de OpenGL
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
    unsigned int indexCount = 0;

    // Metadatos útiles (Bounding Box)
    // AABB aabb; 

    // Carga usando ModelImporter
    void LoadInMemory() override;

    // Limpia buffers de la GPU
    void FreeMemory() override;

    // Helper para dibujar (solo la llamada de dibujo, sin shaders ni transforms)
    void BindAndDraw();
};