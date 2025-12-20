#include "SceneSerializer.h"
#include <fstream>
#include <iostream>
#include "TransformComponent.h"
#include "Application.h"

// --- GUARDAR ---

void SceneSerializer::SaveScene(const std::string& filepath, std::shared_ptr<GameObject> rootObject) {
    json sceneJson;
    sceneJson["name"] = "MyScene";
    sceneJson["gameObjects"] = json::array();

    // Recorremos los hijos del root (asumiendo que el root es invisible/contenedor)
    for (const auto& child : rootObject->GetChildren()) {
        sceneJson["gameObjects"].push_back(SerializeGameObject(child));
    }

    std::ofstream file(filepath);
    file << sceneJson.dump(4); // dump(4) para formato bonito (indentación)
    file.close();
    std::cout << "[Serializer] Escena guardada en: " << filepath << std::endl;
}

json SceneSerializer::SerializeGameObject(std::shared_ptr<GameObject> go) {
    json j;
    j["name"] = go->GetName();
    // j["uid"] = go->GetUID(); // Si tienes UID en GameObject, guárdalo
    j["active"] = go->IsActive();

    // 1. Transform
    auto transform = std::dynamic_pointer_cast<TransformComponent>(go->GetComponent(ComponentType::TRANSFORM));
    if (transform) {
        glm::vec3 pos = transform->GetPosition();
        glm::vec3 rot = transform->GetEulerAngles(); // <--- CAMBIO AQUÍ (antes era GetRotation)
        glm::vec3 scale = transform->GetScale();

        j["components"]["transform"] = {
            {"position", {pos.x, pos.y, pos.z}},
            {"rotation", {rot.x, rot.y, rot.z}},
            {"scale", {scale.x, scale.y, scale.z}}
        };
    }

    // 2. Mesh Renderer (Ejemplo)
    // Aquí deberías guardar la ruta al asset del mesh para poder volver a cargarlo
    /*
    auto meshRenderer = std::dynamic_pointer_cast<RenderMeshComponent>(go->GetComponent(ComponentType::MESH_RENDERER));
    if (meshRenderer && meshRenderer->GetMesh()) {
         j["components"]["meshRenderer"] = {
             {"assetPath", meshRenderer->GetMesh()->GetAssetsPath()}
         };
    }
    */

    // 3. Recursividad para hijos
    j["children"] = json::array();
    for (const auto& child : go->GetChildren()) {
        j["children"].push_back(SerializeGameObject(child));
    }

    return j;
}

// --- CARGAR ---

void SceneSerializer::LoadScene(const std::string& filepath, std::shared_ptr<GameObject> rootObject) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cout << "[Error] No se pudo abrir la escena: " << filepath << std::endl;
        return;
    }

    json sceneJson;
    file >> sceneJson;

    // Limpiar escena actual (Opcional, cuidado con borrar el root)
    // rootObject->ClearChildren(); 

    // Reconstruir
    if (sceneJson.contains("gameObjects")) {
        for (const auto& goJson : sceneJson["gameObjects"]) {
            DeserializeGameObject(goJson, rootObject);
        }
    }
}

void SceneSerializer::DeserializeGameObject(const json& j, std::shared_ptr<GameObject> parent) {
    std::string name = j.value("name", "GameObject");

    // Crear objeto
    // Nota: Necesitas acceso a tu factoría o crear el objeto directamente
    // Como Application::GetInstance().openGL... o similar.
    // Asumiré que puedes crear un GO vacío y asignarle padre.

    auto newGO = std::make_shared<GameObject>(name);
    newGO->SetParent(parent);
    parent->AddChild(newGO); // Importante vincularlo

    if (j.contains("active")) newGO->SetActive(j["active"]);

    // Recuperar Componentes
    if (j.contains("components")) {
        auto components = j["components"];

        // Transform
        if (components.contains("transform")) {
            auto t = components["transform"];
            auto transComp = std::dynamic_pointer_cast<TransformComponent>(newGO->GetComponent(ComponentType::TRANSFORM));
            if (!transComp) transComp = std::dynamic_pointer_cast<TransformComponent>(newGO->AddComponent(ComponentType::TRANSFORM));

            if (transComp) {
                glm::vec3 pos(t["position"][0], t["position"][1], t["position"][2]);
                glm::vec3 rot(t["rotation"][0], t["rotation"][1], t["rotation"][2]);
                glm::vec3 scale(t["scale"][0], t["scale"][1], t["scale"][2]);

                transComp->SetPosition(pos);
                transComp->SetRotation(rot); // Asegúrate de que SetRotation acepte Euler
                transComp->SetScale(scale);
            }
        }

        // Mesh Renderer
        // if (components.contains("meshRenderer")) { ... Cargar Mesh usando ResMan ... }
    }

    // Recursividad Hijos
    if (j.contains("children")) {
        for (const auto& childJson : j["children"]) {
            DeserializeGameObject(childJson, newGO);
        }
    }
}