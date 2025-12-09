
#include <typeinfo>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include "ResMan.h"
#include "FileSystem.cpp"

//hacer save system propio  aqui 


ResourceManager& ResourceManager::GetInstance() {
    static ResourceManager instance;
    return instance;
}

// Constructor (Llama a la configuración inicial de las carpetas)
ResourceManager::ResourceManager() {
    
}



// ---------------------------------------------------------------------
// 2. Lógica Interna de Carga y Cacheo
// ---------------------------------------------------------------------

std::shared_ptr<Resource> ResourceManager::InternalLoad(const std::string& path, const std::string& typeName) {
    // 1. Verificar Cache (CACHE HIT)
    if (m_resources.count(path)) {
        std::cout << "CACHE HIT: Devolviendo " << typeName << " existente en " << path << std::endl;
        return m_resources.at(path);
    }

    // 2. No encontrado: crear nuevo recurso (CACHE MISS)
    std::shared_ptr<Resource> newResource = nullptr;
    // --- Lógica de Factory (Creación de la clase concreta) ---
    if (typeName.find("Mesh") != std::string::npos) {
        newResource = std::make_shared<Mesh>();
    }
    else {
        std::cerr << "ERROR: Tipo de recurso no soportado: " << typeName << std::endl;
        return nullptr;
    }

    // 3. Cargar datos desde Library y añadir a la caché
    if (newResource) {
        newResource->SetPath(path);
        // La ruta 'path' debe apuntar al archivo optimizado en Library/
        newResource->Load();
        m_resources[path] = newResource;
        std::cout << "CACHE MISS: Recurso " << typeName << " CREADO y CARGADO en " << path << std::endl;
    }

    return newResource;
}

// ---------------------------------------------------------------------
// 3. Destructor y Limpieza de Recursos
// ---------------------------------------------------------------------

void ResourceManager::CleanUp() {
    if (!m_resources.empty()) {
        std::cout << "GESTIÓN DE RECURSOS: Limpiando " << m_resources.size() << " recursos en caché." << std::endl;
        m_resources.clear();
    }
}

ResourceManager::~ResourceManager() {
    CleanUp();
    std::cout << "GESTIÓN DE RECURSOS: Gestor destruido." << std::endl;
}


//to do
//● Los modelos, las mallas y las texturas se guardan en vuestro formato propio en la carpeta
//“Library” - La escena se puede serializar a un fichero que se puede cargar. ESTO TB SE PUEDE PONER EN RESMAN TAMPOCO afeCta tanto
//
// https://doc.qt.io/archives/qt-5.11/qtcore-serialization-savegame-example.html
// 
// yo lo dejaria en json pero se puede mirar ig
// 
//● Manejo de recursos :
// 
//○ Hay una ventana de “Assets” que le muestra al usuario los assets.Puede ser un
//simple visualizador de árbol.
// Añadir en gui otra ventana como la de la irquierda
// 
//○ El usuario puede drop / importar nuevos ficheros.La ventana de assets reacciona
//acorde. ESTO SE PUEDE PONER EN RESMAN
// 
//○ Los ficheros de assets se pueden borrar a través de la ventana y los
//correspondientes ficheros de / Library también se eliminan x2
// ESTO TAMBIEN SE PUEDE PONER AQUI SI SE LINKEA AL GUI
// 
//○ Durante el inicio del motor, todos los recursos no manejados aún en assets se
//crean en Library / .SUPONGO Q LA CASA
// 
//○ Todos los recursos cuentan las referencias(e.g.una textura / malla está solo una
//    vez en memoria sin importar cuantos GameObjects la usen).Ese proceso debería
//    poder visualizarse en el editor ESTO? UN POCO XD