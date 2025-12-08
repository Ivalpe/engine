
#include <typeinfo>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include "ResMan.h"

// Usamos el namespace std::filesystem para las operaciones de archivo
namespace fs = std::filesystem;

// Constantes de Rutas
const fs::path ASSETS_PATH = "Assets";
// DEFINICIÓN CLAVE: LIBRARY_PATH = "Assets/Library"
const fs::path LIBRARY_PATH = ASSETS_PATH / "Library";

// NOTA: Se asume que las clases Resource, Mesh, Texture, etc., están definidas en ResMan.h
// y que existe una clase Mesh (o similar) para el InternalLoad.

// ---------------------------------------------------------------------
// 1. Singleton y Constructor
// ---------------------------------------------------------------------

ResourceManager& ResourceManager::GetInstance() {
    static ResourceManager instance;
    return instance;
}

// Constructor (Llama a la configuración inicial de las carpetas)
ResourceManager::ResourceManager() {
    // Asegura que la carpeta Library exista y maneja la regeneración inicial
    CheckAndSetupLibrary();
}

// ---------------------------------------------------------------------
// 1.5. Lógica de Inicialización y Regeneración
// ---------------------------------------------------------------------

/**
 * @brief Recorre Assets/ y, para cada archivo, crea un archivo .meta si no existe
 * y (simula) la creación del recurso binario en Library/.
 */
void ResourceManager::RegenerateLibrary() {
    std::cout << "REGENERACIÓN: Iniciando proceso de regeneración de Library/." << std::endl;

    // Recorrer de forma recursiva la carpeta Assets/
    for (const auto& entry : fs::recursive_directory_iterator(ASSETS_PATH)) {

        // 🚨 LÓGICA DE EXCLUSIÓN: NECESARIA al tener Library/ dentro de Assets/ 🚨
        // Si la ruta del archivo que estamos escaneando empieza con LIBRARY_PATH, lo saltamos.
        if (entry.path().string().find(LIBRARY_PATH.string()) == 0) {
            continue;
        }

        if (entry.is_regular_file()) {
            const fs::path assetPath = entry.path();

            // 1. Manejar el archivo .meta asociado
            fs::path metaPath = assetPath;
            metaPath += ".meta";

            if (!fs::exists(metaPath)) {
                // Crear un nuevo archivo .meta (simulación)
                std::ofstream metaFile(metaPath);
                metaFile << "version: 1" << "\n";
                metaFile << "import_settings: default";
                metaFile.close();
                std::cout << "  [META] Creado: " << metaPath.string() << std::endl;
            }

            // 2. Construir la ruta en Library/ 
            std::string assetPathStr = assetPath.string();
            // Calcula la ruta relativa a "Assets/"
            std::string assetRelativePath = assetPathStr.substr(ASSETS_PATH.string().size());

            // Eliminar el '/' inicial si existe
            if (!assetRelativePath.empty() && assetRelativePath.front() == fs::path::preferred_separator) {
                assetRelativePath.erase(0, 1);
            }

            // Construimos la ruta dentro de Assets/Library/
            fs::path libraryItemPath = LIBRARY_PATH / assetRelativePath;

            // Asegurar que la subcarpeta en Library/ exista
            fs::path libraryDir = libraryItemPath.parent_path();
            if (!fs::exists(libraryDir)) {
                fs::create_directories(libraryDir);
            }

            // Simulación de la "importación" (Creación del archivo binario optimizado)
            fs::path libraryItemFinalPath = libraryItemPath.string() + ".bin";

            if (!fs::exists(libraryItemFinalPath)) {
                std::ofstream libFile(libraryItemFinalPath);
                libFile << "DATOS BINARIOS COMPRIMIDOS DEL RECURSO";
                libFile.close();

                std::cout << "  [IMPORT] Procesado: " << assetPath.string()
                    << " -> Creado en " << libraryItemFinalPath.string() << std::endl;
            }
            else {
                // Lógica de motor: Aquí se verificaría si el .meta o el asset original ha cambiado
                std::cout << "  [SKIP] Archivo en Library/ ya existe: " << libraryItemFinalPath.string() << std::endl;
            }
        }
    }
    std::cout << "REGENERACIÓN: Proceso de regeneración finalizado." << std::endl;
}

/**
 * @brief Comprueba y crea la carpeta Library. Si es necesario, la regenera.
 */
void ResourceManager::CheckAndSetupLibrary() {
    std::cout << "GESTIÓN DE RECURSOS: Verificando carpetas Assets/ y " << LIBRARY_PATH.string() << "..." << std::endl;

    // 2. Comprobar y Crear Library/
    if (!fs::exists(LIBRARY_PATH)) {
        std::cout << "INFO: La ruta " << LIBRARY_PATH.string() << " no existe. Creándola y regenerando recursos..." << std::endl;

        // fs::create_directories crea todas las carpetas necesarias ("Assets" y "Assets/Library")
        if (fs::create_directories(LIBRARY_PATH)) {
            std::cout << "INFO: Creada la ruta completa: " << LIBRARY_PATH.string() << std::endl;
            RegenerateLibrary(); // Llama a la lógica de regeneración
        }
        else {
            std::cerr << "ERROR: No se pudo crear el directorio " << LIBRARY_PATH.string() << ". Verifique permisos." << std::endl;
        }
    }
    else {
        std::cout << "INFO: La ruta " << LIBRARY_PATH.string() << " existe. Listo para cargar recursos." << std::endl;
    }
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
// 
//○ Durante el inicio del motor, todos los recursos no manejados aún en assets se
//crean en Library / .SUPONGO Q LA CASA
// 
//○ La carpeta / Library debería regenerarse desde la carpeta / Assets mas los
//contenidos del fichero.meta ESTO NO VA POR LA CARA
// 
//○ Todos los recursos cuentan las referencias(e.g.una textura / malla está solo una
//    vez en memoria sin importar cuantos GameObjects la usen).Ese proceso debería
//    poder visualizarse en el editor ESTO? UN POCO XD