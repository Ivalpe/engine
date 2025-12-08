#pragma once
#include "Module.h"
#include "Resource.h"
#include <unordered_map>
#include <memory>
#include <string>

class ResourceManager : public Module {
public:
    ResourceManager();
    ~ResourceManager();

    static ResourceManager& GetInstance();

    bool Awake() override;
    bool CleanUp() override;

    // Método principal para obtener recursos
    // Si ya está en memoria, devuelve el puntero. Si no, lo carga.
    std::shared_ptr<Resource> GetResource(uint32_t uid);

    // Método para crear un NUEVO recurso (importación)
    std::shared_ptr<Resource> CreateResource(ResourceType type, uint32_t uid, const std::string& assetsPath);

private:
    // Mapa de UID -> Recurso cargado
    std::unordered_map<uint32_t, std::shared_ptr<Resource>> resources;
};