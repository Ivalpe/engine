#pragma once
#include "ResourceMesh.h"
#include <vector>
#include <string>

namespace ModelImporter {
    // Importa un archivo (fbx) y genera archivos .vroom en Library
    bool Import(const char* filePath, const char* destDir);

    // Carga un archivo .vroom desde Library al Recurso
    bool Load(ResourceMesh* resource);

    // Función auxiliar para guardar formato propio
    void SaveToLibrary(const char* path, uint32_t numVerts, uint32_t numIndices, const float* vertices, const unsigned int* indices);
}