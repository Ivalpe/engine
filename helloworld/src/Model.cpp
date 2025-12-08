#include "Model.h"
#include "ModelImporter.h"
#include "Application.h"
#include "ResMan.h"
#include "GameObject.h"
#include "RenderMeshComponent.h"
#include "TransformComponent.h"
#include "MaterialComponent.h"
#include "Log.h"
#include <filesystem>
#include <algorithm>

using namespace std;

Model::Model(std::string path) {
    loadModel(path);
}

Model::Model() {}

Model::~Model() {
    gameObjects.clear();
    meshes.clear();
}

void Model::loadModel(std::string path) {
    fullPath = path;
    std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

    fileName = std::filesystem::path(path).filename().string();
    directory = std::filesystem::path(path).parent_path().string();

    LOG("Loading Model via Importer: %s", fullPath.c_str());

    // 1. IMPORTACIÓN: Generar archivos binarios .vroom en Library
    std::string libDir = "Library/Meshes";
    // Nota: En un motor real, Import solo se llamaría si el archivo .meta ha cambiado
    // o si el binario no existe. Por ahora, lo llamamos siempre para asegurar.
    if (!ModelImporter::Import(path.c_str(), libDir.c_str())) {
        LOG("Failed to import model: %s", path.c_str());
        return;
    }

    // 2. CONSTRUCCIÓN DE ESCENA (GameObject Hierarchy)
    // Como ModelImporter (por ahora) solo guarda meshes sueltas y no un archivo de escena completo,
    // vamos a simular la creación de un GameObject por cada mesh generada.

    rootGameObject = std::make_shared<GameObject>(fileName);
    rootGameObject->SetOwnerModel(this);
    gameObjects.push_back(rootGameObject);

    // HACK: Asumimos que ModelImporter ha generado archivos con sufijo _mesh_X.vroom
    // Intentamos cargar "a ciegas" unos cuantos índices o hasta fallar.
    // EN EL FUTURO: ModelImporter::Import debería devolver una lista de UIDs o paths generados.

    int meshIndex = 0;
    while (true) {
        // Construir el nombre esperado en Library
        std::string stem = std::filesystem::path(path).stem().string();
        std::string meshLibName = stem + "_mesh_" + std::to_string(meshIndex) + ".vroom";
        std::string meshLibPath = libDir + "/" + meshLibName;

        if (!std::filesystem::exists(meshLibPath)) {
            if (meshIndex == 0) LOG("Warning: No meshes found for %s", path.c_str());
            break; // No hay más meshes
        }

        // Crear GameObject hijo
        auto childGO = std::make_shared<GameObject>(stem + "_Mesh_" + std::to_string(meshIndex));
        childGO->SetParent(rootGameObject);
        childGO->SetOwnerModel(this);

        // Añadir RenderMeshComponent
        auto renderComp = std::dynamic_pointer_cast<RenderMeshComponent>(childGO->AddComponent(ComponentType::MESH_RENDERER));

        // Crear/Cargar Recurso Mesh
        // Usamos un UID hash simple del path para identificarlo
        uint32_t meshUID = std::hash<std::string>{}(meshLibPath);

        // Pedir al ResourceManager
        auto resMesh = std::dynamic_pointer_cast<ResourceMesh>(
            ResourceManager::GetInstance().CreateResource(ResourceType::MESH, meshUID, path)
        );

        // Asignar ruta binaria y cargar
        resMesh->libraryPath = meshLibPath;
        resMesh->LoadInMemory();

        // Asignar al componente
        renderComp->SetMesh(resMesh);
        meshes.push_back(resMesh);

        // Añadir MaterialComponent (Placeholder)
        childGO->AddComponent(ComponentType::MATERIAL);

        gameObjects.push_back(childGO);
        meshIndex++;
    }

    LOG("Model loaded. Created %d GameObjects from meshes.", meshIndex);
}

void Model::Draw(Shader& shader) {
    // Este método ya no es necesario si el sistema de Render recorre los componentes,
    // pero lo mantenemos por compatibilidad si tu Render llama a Model::Draw
    for (auto& go : gameObjects) {
        auto meshRenderer = std::dynamic_pointer_cast<RenderMeshComponent>(go->GetComponent(ComponentType::MESH_RENDERER));
        if (meshRenderer) {
            meshRenderer->Render(&shader);
        }
    }
}

void Model::DestroyGameObject(std::shared_ptr<GameObject> gameObject) {
    if (gameObject) gameObject->MarkForDestroy();
}

void Model::CleanUpDestroyedObjects() {
    auto it = std::remove_if(gameObjects.begin(), gameObjects.end(),
        [](const std::shared_ptr<GameObject>& obj) { return obj->IsMarkedForDestroy(); });

    if (it != gameObjects.end()) {
        gameObjects.erase(it, gameObjects.end());
    }
}

void Model::LogGameObjectHierarchy(std::shared_ptr<GameObject> go, int depth) {
    if (!go) return;
    std::string indent(depth * 2, ' ');
    LOG("%s- %s", indent.c_str(), go->GetName().c_str());
    for (const auto& child : go->GetChildren()) {
        LogGameObjectHierarchy(child, depth + 1);
    }
}