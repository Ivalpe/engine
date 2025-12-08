#pragma once
#include <string>
#include "ResourceTexture.h"

namespace TextureImporter {
    // Importa una textura (png, jpg, etc) y crea el archivo binario propio
    bool Import(const char* filePath, const char* destDir);

    // Carga el archivo binario propio a la GPU
    bool Load(ResourceTexture* resource);

    // Función auxiliar para guardar en formato propio
    void SaveToLibrary(const char* path, int width, int height, int channels, const void* data);
}