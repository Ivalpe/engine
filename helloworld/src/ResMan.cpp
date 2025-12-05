#include "ResourceManager.h"
#include <typeinfo> 
#include <iostream>

//Añadir includes de recursos concretos + el selection (a lo mejor en otro file?)

ResourceManager& ResourceManager::GetInstance() {
    // La instancia se crea la primera vez que se llama a esta función.
    // Es thread-safe desde C++11.
    static ResourceManager instance;
    return instance;
}

// ---------------------------------------------------------------------
// 2. Lógica Interna de Carga y Cacheo
// ---------------------------------------------------------------------

/**
 * @brief Lógica central para verificar el caché, crear el recurso si es necesario y cargarlo.
 * * @param path La ruta del archivo en la carpeta "Library".
 * @param typeName El nombre del tipo de recurso solicitado (ej: "Mesh", "Texture").
 * @return std::shared_ptr<Resource> Puntero al recurso cargado o nullptr si falla.
 */
std::shared_ptr<Resource> ResourceManager::InternalLoad(const std::string& path, const std::string& typeName) {
    // 1. Verificar Cache (CACHE HIT)
    if (m_resources.count(path)) {
        std::cout << "CACHE HIT: Devolviendo " << typeName << " existente en " << path << std::endl;
        return m_resources.at(path);
    }

    // 2. No encontrado: crear nuevo recurso (CACHE MISS)
    std::shared_ptr<Resource> newResource = nullptr;

    // --- Lógica de Factory (Creación de la clase concreta) ---
    // NOTA: typeName viene del typeid(T).name(), que puede ser mangled. 
    // Usamos .find() para ser más robustos en la prueba.

    // En un motor real, usarías una forma más fiable que typeid(T).name() 
    // para la creación de la instancia, como un sistema de Factory registrado.

    // Aquí usamos Mesh como ejemplo.
    if (typeName.find("Mesh") != std::string::npos) {
        newResource = std::make_shared<Mesh>();
    }
    // else if (typeName.find("Texture") != std::string::npos) {
    //     newResource = std::make_shared<Texture>();
    // } 
    else {
        std::cerr << "ERROR: Tipo de recurso no soportado: " << typeName << std::endl;
        return nullptr;
    }

    // 3. Cargar datos desde Library y añadir a la caché
    if (newResource) {
        newResource->SetPath(path);

        // Llamada a la carga específica (Mesh::Load(), Texture::Load(), etc.)
        newResource->Load();

        m_resources[path] = newResource;
        std::cout << "CACHE MISS: Recurso " << typeName << " CREADO y CARGADO en " << path << std::endl;
    }

    return newResource;
}

// ---------------------------------------------------------------------
// 3. Destructor y Limpieza de Recursos
// ---------------------------------------------------------------------

/**
 * @brief Limpia explícitamente la caché de recursos.
 */
void ResourceManager::CleanUp() {
    // Al llamar a clear(), el std::unordered_map borra todos sus std::shared_ptr.
    // Cuando el shared_ptr se destruye, si su Reference Count llega a 0, 
    // el destructor del objeto Resource (y de la clase derivada) es llamado.
    if (!m_resources.empty()) {
        std::cout << "GESTIÓN DE RECURSOS: Limpiando " << m_resources.size() << " recursos en caché." << std::endl;
        m_resources.clear();
    }
}

/**
 * @brief Destructor del ResourceManager.
 */
ResourceManager::~ResourceManager() {
    // Aseguramos que la caché se vacíe al salir del programa.
    CleanUp();
    std::cout << "GESTIÓN DE RECURSOS: Gestor destruido." << std::endl;
}