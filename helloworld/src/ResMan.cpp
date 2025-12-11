
#include <typeinfo>
#include <iostream>
#include <filesystem>
#include "Application.h"
#include <fstream>
#include <string>
#include "ResMan.h"
#include "FileSystem.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "stb_image.h"

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
    // AQUÍ DEBES AÑADIR TEXTURAS SI QUIERES QUE FUNCIONE CON ELLAS TAMBIÉN
    else if (typeName.find("Texture") != std::string::npos) {
        // newResource = std::make_shared<Texture>(); 
        // Asegúrate de tener el constructor adecuado en Texture
    }
    else {
        std::cerr << "ERROR: Tipo de recurso no soportado: " << typeName << std::endl;
        return nullptr;
    }

    // 3. Cargar datos desde Library y añadir a la caché
    if (newResource) {
        // A) Obtener o Generar el UUID del archivo original
        VroomUUID uid = GetOrCreateMeta(path);

        // B) Construir la ruta al archivo binario en Library
        std::string libPath = "Assets/Library/" + std::to_string(uid);

        // C) Asegurarnos de que el archivo binario existe
        // Si no existe (porque acabas de meter el fbx), lo creamos ahora mismo.
        bool libExists = Application::GetInstance().fileSystem->Exists(libPath);
        if (!libExists) {
            std::cout << "[InternalLoad] Generando binario en Library para: " << path << std::endl;
            SaveToLibrary(path, uid);
        }

        // D) Configurar el recurso
        newResource->SetUID(uid);
        newResource->SetAssetsPath(path);    // Guardamos la ruta original (Assets/...)
        newResource->SetLibraryPath(libPath); // Guardamos la ruta binaria (Library/...)

        // E) Cargar (Ahora Mesh::Load usará 'GetLibraryPath' para leer el binario)
        newResource->Load();

        // F) Guardar en el mapa de recursos
        m_resources[path] = newResource;
        std::cout << "CACHE MISS: Recurso " << typeName << " CREADO y CARGADO desde Library (" << uid << ")" << std::endl;
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
    // Ruta destino: Assets/Library/UUID
    std::string libPath = "Assets/Library/" + std::to_string(uid);

    // Detectar extensión para saber cómo procesar
    std::string extension = assetPath.substr(assetPath.find_last_of(".") + 1);

    // Convertir a minúsculas por si acaso (FBX, fbx)
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    if (extension == "png" || extension == "jpg" || extension == "jpeg" || extension == "tga") {
        ImportTexture(assetPath, libPath);
    }
    else if (extension == "fbx" || extension == "obj") {
        ImportMesh(assetPath, libPath);
    }
    else {
        // Fallback: copia directa para archivos desconocidos
        Application::GetInstance().fileSystem->Copy(assetPath, libPath);
    }
}

void ResourceManager::ImportTexture(const std::string& assetPath, const std::string& libPath) {
    int width, height, channels;
    // Cargar píxeles crudos (sin comprimir)
    unsigned char* data = stbi_load(assetPath.c_str(), &width, &height, &channels, 0);

    if (data) {
        std::ofstream file(libPath, std::ios::binary);
        if (file.is_open()) {
            // CABECERA: Guardamos ancho, alto y canales
            file.write((char*)&width, sizeof(int));
            file.write((char*)&height, sizeof(int));
            file.write((char*)&channels, sizeof(int));

            // DATOS: Guardamos el array de píxeles completo
            // Tamaño = w * h * bytes_por_canal (char = 1 byte)
            file.write((char*)data, width * height * channels);

            file.close();
            std::cout << "[Import] Textura guardada en Library: " << libPath << std::endl;
        }
        stbi_image_free(data);
    }
    else {
        std::cerr << "[Error] No se pudo cargar textura para importar: " << assetPath << std::endl;
    }
}

void ResourceManager::ImportMesh(const std::string& assetPath, const std::string& libPath) {
    Assimp::Importer importer;
    // Importante: Triangulate para asegurar triángulos, FlipUVs para texturas
    const aiScene* scene = importer.ReadFile(assetPath, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || !scene->mRootNode || scene->mNumMeshes == 0) {
        std::cerr << "[Error] Assimp no pudo cargar el mesh: " << assetPath << std::endl;
        return;
    }

    // SIMPLIFICACIÓN: Por ahora guardamos solo la PRIMERA malla encontrada en el archivo.
    // Un sistema más complejo iteraría sobre 'scene->mMeshes' y guardaría múltiples archivos o un archivo compuesto.
    aiMesh* mesh = scene->mMeshes[0];

    std::ofstream file(libPath, std::ios::binary);
    if (file.is_open()) {

        // 1. HEADER
        uint32_t numVertices = mesh->mNumVertices;
        uint32_t numIndices = 0;

        // Calcular total de índices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            numIndices += mesh->mFaces[i].mNumIndices;
        }

        file.write((char*)&numVertices, sizeof(uint32_t));
        file.write((char*)&numIndices, sizeof(uint32_t));

        // 2. BODY - Vertices
        // Tu struct Vertex tiene: vec3 Position, vec3 Normal, vec2 TexCoord
        for (unsigned int i = 0; i < numVertices; i++) {
            // Pos
            float pos[3] = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
            file.write((char*)pos, sizeof(float) * 3);

            // Norm
            float norm[3] = { 0,0,0 };
            if (mesh->HasNormals()) {
                norm[0] = mesh->mNormals[i].x;
                norm[1] = mesh->mNormals[i].y;
                norm[2] = mesh->mNormals[i].z;
            }
            file.write((char*)norm, sizeof(float) * 3);

            // TexCoord
            float tex[2] = { 0,0 };
            if (mesh->HasTextureCoords(0)) {
                tex[0] = mesh->mTextureCoords[0][i].x;
                tex[1] = mesh->mTextureCoords[0][i].y;
            }
            file.write((char*)tex, sizeof(float) * 2);
        }

        // 3. BODY - Indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                uint32_t idx = face.mIndices[j];
                file.write((char*)&idx, sizeof(uint32_t));
            }
        }

        file.close();
        std::cout << "[Import] Mesh guardada en Library: " << libPath << std::endl;
    }
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