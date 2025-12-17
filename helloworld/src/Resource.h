#pragma once
#include <string>
#include "UUID.h" // Asegúrate de tener tu UUID.h

enum class ResourceType {
    UNKNOWN,
    MESH,
    TEXTURE,
    MODEL
};

class Resource {
public:
    // Constructor
    Resource(ResourceType type, const std::string& fileName)
        : type(type), fileName(fileName), uid(0) {
    }

    virtual ~Resource() = default;

    // Métodos virtuales
    virtual void Load() {} // Cada hijo (Mesh, Texture) implementará el suyo

    // Getters y Setters
    void SetAssetsPath(const std::string& p) { assetsPath = p; }
    const std::string& GetAssetsPath() const { return assetsPath; }

    void SetLibraryPath(const std::string& p) { libraryPath = p; }
    const std::string& GetLibraryPath() const { return libraryPath; }

    void SetUID(VroomUUID id) { uid = id; }
    VroomUUID GetUID() const { return uid; }

    ResourceType GetType() const { return type; }
    const std::string& GetName() const { return fileName; }

protected:
    VroomUUID uid;
    std::string assetsPath;  // Ruta original (Assets/...)
    std::string libraryPath; // Ruta procesada (Library/UID)
    std::string fileName;
    ResourceType type;
};