#include "ResourceMesh.h"
#include "ModelImporter.h" 
#include "Log.h"

ResourceMesh::ResourceMesh(uint32_t uid) : Resource(uid, ResourceType::MESH) {
    // Inicializamos las variables para evitar basura en memoria
    indexCount = 0;
    vertexCount = 0;
    VAO = 0;
    VBO = 0;
    EBO = 0;
}

ResourceMesh::~ResourceMesh() {
    FreeMemory();
}

void ResourceMesh::LoadInMemory() {
    if (loaded) return;

    // Delegamos la carga al Importer
    if (ModelImporter::Load(this)) {
        loaded = true;
    }
    else {
        LOG("Error loading mesh resource with UID: %d", UID);
    }
}

void ResourceMesh::FreeMemory() {
    if (!loaded) return;

    // Borramos buffers de la GPU
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);

    // Reseteamos IDs y contadores
    VAO = 0;
    VBO = 0;
    EBO = 0;
    indexCount = 0;
    vertexCount = 0; // Importante resetear esto también

    loaded = false;
}

void ResourceMesh::BindAndDraw() {
    if (!loaded) return;

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}