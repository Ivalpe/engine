#include "ResourceMesh.h"
#include "ModelImporter.h" // Asumimos que has creado este archivo como hablamos
#include "Log.h"

ResourceMesh::ResourceMesh(uint32_t uid) : Resource(uid, ResourceType::MESH) {
}

ResourceMesh::~ResourceMesh() {
    FreeMemory();
}

void ResourceMesh::LoadInMemory() {
    if (loaded) return;

    // Delegamos la carga al Importer. 
    // El Importer leerá el archivo .vroom de 'libraryPath' y llenará VAO/VBO/EBO
    if (ModelImporter::Load(this)) {
        loaded = true;
    }
    else {
        LOG("Error loading mesh resource with UID: %d", UID);
    }
}

void ResourceMesh::FreeMemory() {
    if (!loaded) return;

    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);

    VAO = 0;
    VBO = 0;
    EBO = 0;
    indexCount = 0;
    loaded = false;
}

void ResourceMesh::BindAndDraw() {
    if (!loaded) return;

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}