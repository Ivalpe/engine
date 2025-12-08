#pragma once
#include "Resource.h"
#include <SDL3/SDL_opengl.h>
#include <glm/glm.hpp>

// Definimos la estructura Vertex
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class ResourceMesh : public Resource {
public:
    ResourceMesh(uint32_t uid);
    ~ResourceMesh();

    // Datos de OpenGL
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;

    // Contadores
    unsigned int indexCount = 0;
    unsigned int vertexCount = 0;

    // Métodos (Solo declaraciones)
    void LoadInMemory() override;
    void FreeMemory() override;
    void BindAndDraw();
};