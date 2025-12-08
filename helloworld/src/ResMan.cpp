#include "ResMan.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "Log.h"

ResourceManager::ResourceManager() : Module() {
    name = "ResourceManager";
}

ResourceManager::~ResourceManager() {}

ResourceManager& ResourceManager::GetInstance() {
    static ResourceManager instance;
    return instance;
}

bool ResourceManager::Awake() {
    return true;
}

bool ResourceManager::CleanUp() {
    resources.clear();
    return true;
}

std::shared_ptr<Resource> ResourceManager::GetResource(uint32_t uid) {
    // 1. Buscar en Caché
    auto it = resources.find(uid);
    if (it != resources.end()) {
        // Encontrado: Aumentar referencias y devolver
        it->second->IncreaseReferenceCount();
        return it->second;
    }

    // 2. Si no está en caché, deberíamos buscar en Library si existe el archivo .vroom
    // IMPORTANTE: Aquí necesitas saber qué TIPO de recurso es ese UID. 
    // Normalmente tendrías un mapa "UID -> Info" (obtenido de cargar los .meta al inicio).
    // Como simplificación por ahora, retornamos nullptr si no se creó explícitamente.

    LOG("Resource with UID %d not found in Cache.", uid);
    return nullptr;
}

std::shared_ptr<Resource> ResourceManager::CreateResource(ResourceType type, uint32_t uid, const std::string& assetsPath) {
    std::shared_ptr<Resource> newRes = nullptr;

    // Factory básica
    switch (type) {
    case ResourceType::MESH:
        newRes = std::make_shared<ResourceMesh>(uid);
        break;
    case ResourceType::TEXTURE:
        newRes = std::make_shared<ResourceTexture>(uid);
        break;
        // Case Model, Scene, etc.
    }

    if (newRes) {
        newRes->assetsPath = assetsPath;
        // Aquí deberías asignar el libraryPath basado en tu lógica de Importer
        // Ej: newRes->libraryPath = "Library/Meshes/" + std::to_string(uid) + ".vroom";

        resources[uid] = newRes;
    }

    return newRes;
}