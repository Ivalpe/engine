#pragma once
#include <string>
#include <memory>
#include "GameObject.h"

// Necesitas nlohmann/json. Si usas vcpkg: "nlohmann-json"
#include <nlohmann/json.hpp> 

using json = nlohmann::json;

class SceneSerializer {
public:
    // Guardar la escena actual a un archivo
    static void SaveScene(const std::string& filepath, std::shared_ptr<GameObject> rootObject);

    // Cargar una escena desde un archivo
    static void LoadScene(const std::string& filepath, std::shared_ptr<GameObject> rootObject);

private:
    // Métodos auxiliares
    static json SerializeGameObject(std::shared_ptr<GameObject> go);
    static void DeserializeGameObject(const json& j, std::shared_ptr<GameObject> parent);
};