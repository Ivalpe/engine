
#include <typeinfo>
#include <iostream>
#include <filesystem>
#include "Application.h"
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

// ---------------------------------------------------------------------
// IMPORTACIÓN DE ASSETS Y GESTIÓN DE META-DATA
// ---------------------------------------------------------------------

void ResourceManager::ImportAssets() {
    LOG("Resource Manager: Importando Assets...");

    // 1. Acceder al FileSystem
    auto fs = Application::GetInstance().fileSystem;

    // 2. Obtener todos los archivos de Assets (recursivamente)
    std::vector<std::string> files = fs->GetAllFiles("Assets", true);

    for (const std::string& path : files) {
        // Ignoramos los propios archivos .meta para no entrar en bucle infinito
        if (path.find(".meta") != std::string::npos) continue;

        // 3. Obtener el UUID (leyendo el .meta o creando uno nuevo)
        VroomUUID uid = GetOrCreateMeta(path);

        // 4. Verificar si el archivo ya existe en la Library
        // En la Library, el archivo se llamará simplemente como el número UUID
        std::string libraryPath = "Assets/Library/" + std::to_string(uid);

        bool libraryExists = fs->Exists(libraryPath);

        // Si no existe en Library, lo "importamos" (por ahora copiamos)
        if (!libraryExists) {
            std::cout << "[Import] Generando recurso en Library: " << path << " -> " << uid << std::endl;
            SaveToLibrary(path, uid);
        }
    }
}

VroomUUID ResourceManager::GetOrCreateMeta(const std::string& path) {
    std::string metaPath = path + ".meta";
    auto fs = Application::GetInstance().fileSystem;

    // A) CASO 1: El .meta YA EXISTE. Lo leemos.
    if (fs->Exists(metaPath)) {
        std::ifstream file(metaPath);
        std::string line;
        VroomUUID uid = 0;
        if (file.is_open()) {
            while (std::getline(file, line)) {
                if (line.find("UID: ") != std::string::npos) {
                    // Extraemos el número después de "UID: "
                    try {
                        // std::stoull convierte string a unsigned long long (nuestro UUID)
                        uid = std::stoull(line.substr(5));
                    }
                    catch (...) {
                        uid = 0;
                    }
                    break;
                }
            }
            file.close();
            // Si leímos un ID válido, lo devolvemos
            if (uid != 0) return uid;
        }
    }

    // B) CASO 2: El .meta NO EXISTE. Creamos uno nuevo.
    VroomUUID newUID = UUIDGen::GenerateUUID(); // <--- Usamos tu clase aquí

    std::ofstream file(metaPath);
    if (file.is_open()) {
        file << "UID: " << newUID << "\n";
        // Guardamos fecha de modificación para futuras comprobaciones
        file << "Time: " << fs->GetLastModTime(path) << "\n";
        file.close();
        // LOG("Meta generado para: %s", path.c_str());
    }

    return newUID;
}

void ResourceManager::SaveToLibrary(const std::string& assetPath, VroomUUID uid) {
    // Definimos la ruta destino: Assets/Library/NUMERO_UUID
    std::string libPath = "Assets/Library/" + std::to_string(uid);

    // Por ahora hacemos una copia directa.
    // TAREA FUTURA: Aquí es donde convertirías el .fbx a tu formato binario propio.
    Application::GetInstance().fileSystem->Copy(assetPath, libPath);
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