#pragma once
#include <string>
#include <iostream>

// Usamos enum class para mayor seguridad de tipos
enum class ResourceType {
    UNKNOWN,
    MESH,
    TEXTURE,
    MODEL,
    SCENE
};

class Resource {
public:
    // UID único para cada recurso (generado al importar)
    Resource(uint32_t uid, ResourceType type) : UID(uid), type(type) {}

    virtual ~Resource() {}

    ResourceType GetType() const { return type; }
    uint32_t GetUID() const { return UID; }

    // Ruta original del archivo (ej: "Assets/BakerHouse.fbx")
    std::string assetsPath;
    // Ruta del archivo binario propio (ej: "Library/Meshes/mesh_123.vroom")
    std::string libraryPath;

    // Carga los datos desde el archivo binario (libraryPath) a la memoria (RAM/VRAM)
    virtual void LoadInMemory() = 0;

    // Libera la memoria (VRAM) si el recurso no se está usando
    virtual void FreeMemory() = 0;

    bool IsLoaded() const { return loaded; }

    // Sistema de conteo de referencias
    void IncreaseReferenceCount() { referenceCount++; }
    void DecreaseReferenceCount() {
        if (referenceCount > 0) referenceCount--;
    }
    uint32_t GetReferenceCount() const { return referenceCount; }

protected:
    uint32_t UID = 0;
    ResourceType type = ResourceType::UNKNOWN;
    uint32_t referenceCount = 0;
    bool loaded = false;
};