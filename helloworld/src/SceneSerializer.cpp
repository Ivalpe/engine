#include "SceneSerializer.h"
#include <fstream>
#include <iostream>
#include "TransformComponent.h"
#include "Application.h"
#include "RenderMeshComponent.h"
#include "MaterialComponent.h"
#include "ResMan.h"
#include "FileSystem.h"

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
        glm::vec3 rot = transform->GetEulerAngles(); 
        glm::vec3 scale = transform->GetScale();

        j["components"]["transform"] = {
            {"position", {pos.x, pos.y, pos.z}},
            {"rotation", {rot.x, rot.y, rot.z}},
            {"scale", {scale.x, scale.y, scale.z}}
        };
    }

    auto meshRenderer = std::dynamic_pointer_cast<RenderMeshComponent>(go->GetComponent(ComponentType::MESH_RENDERER));
    if (meshRenderer && meshRenderer->GetMesh()) {
        

        j["components"]["meshRenderer"] = {
            {"path", meshRenderer->GetMesh()->GetAssetsPath()}
        };
    }

    // 3. Material (Textura)
    auto material = std::dynamic_pointer_cast<MaterialComponent>(go->GetComponent(ComponentType::MATERIAL));
    if (material && material->GetDiffuseMap()) {
        j["components"]["material"] = {
            {"diffusePath", material->GetDiffuseMap()->path}
        };
    }



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

    auto newGO = std::make_shared<GameObject>(name);
    newGO->SetParent(parent);
    parent->AddChild(newGO);

    if (j.contains("active")) newGO->SetActive(j["active"]);

    // Recuperar Componentes
    if (j.contains("components")) {
        auto components = j["components"];

        // --- 1. TRANSFORM ---
        if (components.contains("transform")) {
            auto t = components["transform"];
            auto transComp = std::dynamic_pointer_cast<TransformComponent>(newGO->GetComponent(ComponentType::TRANSFORM));
            if (!transComp) transComp = std::dynamic_pointer_cast<TransformComponent>(newGO->AddComponent(ComponentType::TRANSFORM));

            if (transComp) {
                glm::vec3 pos(t["position"][0], t["position"][1], t["position"][2]);
                glm::vec3 rot(t["rotation"][0], t["rotation"][1], t["rotation"][2]);
                glm::vec3 scale(t["scale"][0], t["scale"][1], t["scale"][2]);

                transComp->SetPosition(pos);
                transComp->SetRotation(rot);
                transComp->SetScale(scale);
            }
        }

        // --- 2. MESH RENDERER (Aquí va lo nuevo) ---
        if (components.contains("meshRenderer")) {
            std::string meshPath = components["meshRenderer"]["path"];

            // --- CAMBIO AQUÍ ---
            // Antes: auto mesh = ResourceManager::GetInstance().Load<Mesh>(meshPath);
            auto mesh = ResourceManager::GetInstance().LoadMesh(meshPath);
            // -------------------

            if (mesh) {
                auto meshComp = std::dynamic_pointer_cast<RenderMeshComponent>(newGO->AddComponent(ComponentType::MESH_RENDERER));
                if (meshComp) meshComp->SetMesh(mesh);
            }
        }

        // --- 3. MATERIAL (Aquí va lo nuevo) ---
        if (components.contains("material")) {
            std::string texPath = components["material"]["diffusePath"];

            auto materialComp = std::dynamic_pointer_cast<MaterialComponent>(newGO->AddComponent(ComponentType::MATERIAL));

            // Si AddComponent ya crea el componente y lo devuelve:
            if (materialComp) {
                auto texture = std::make_shared<Texture>();

                // Extraemos directorio y archivo para tu función TextureFromFile
                std::string filename = FileSystem::GetFileName(texPath);
                // Truco: Si texPath es "Assets/Textures/wall.jpg", dir es "Assets/Textures"
                std::string dir = texPath.substr(0, texPath.find_last_of('/'));

                // Cargar
                texture->TextureFromFile(dir, filename.c_str());
                materialComp->SetDiffuseMap(texture);
            }
        }
    }

    // Recursividad Hijos
    if (j.contains("children")) {
        for (const auto& childJson : j["children"]) {
            DeserializeGameObject(childJson, newGO);
        }
    }
}