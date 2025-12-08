
#pragma once
//include  las cargas y meshes
#include "Model.h"
#include "GameObject.h"
#include "Mesh.h"
#include <string>
#include <memory>         
#include <unordered_map>  

              
        class Resource;
        class Mesh;
        class Texture;
        class Lighting;
        class Shader;
        class Material;

        class ResourceManager {
        public:
            
            static ResourceManager& GetInstance();

            template <typename T>
            std::shared_ptr<T> Load(const std::string& path);

           
            void CleanUp();

        private:
           
            ResourceManager() = default;
            ~ResourceManager();
            ResourceManager(const ResourceManager&) = delete;
            ResourceManager& operator=(const ResourceManager&) = delete;


            std::unordered_map<std::string, std::shared_ptr<Resource>> m_resources;

            // Opcional: Método interno para cargar el recurso (esto es del gm)
            std::shared_ptr<Resource> InternalLoad(const std::string& path, const std::string& typeName);
        };

    