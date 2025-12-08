#pragma once
#include "Shader.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "GameObject.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

class Model {
public:
    Model(std::string path);
    Model();
    ~Model();

    void Draw(Shader& shader); // Método legacy, probablemente ya no se use si GameObjects se pintan solos

    // Lista de recursos usados por este modelo
    std::vector<std::shared_ptr<ResourceMesh>> meshes;
    std::vector<std::shared_ptr<ResourceTexture>> textures;

    // Jerarquía
    std::shared_ptr<GameObject> rootGameObject;
    std::vector<std::shared_ptr<GameObject>> gameObjects; // Lista plana para acceso rápido

    std::vector<std::shared_ptr<GameObject>>& GetGameObjects() { return gameObjects; }
    std::shared_ptr<GameObject> GetRootGameObject() const { return rootGameObject; }

    std::string fileName, fileExtension, directory;
    std::string fullPath;

    // Lógica principal: Cargar usando el Importer y crear GameObjects
    void loadModel(std::string path);

    // Gestión de texturas por defecto
    bool useDefaultTexture = false;
    std::shared_ptr<ResourceTexture> savedTexture;

    void LogGameObjectHierarchy(std::shared_ptr<GameObject> go, int depth);
    void DestroyGameObject(std::shared_ptr<GameObject> gameObject);
    void CleanUpDestroyedObjects();
};