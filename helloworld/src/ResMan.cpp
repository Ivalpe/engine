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
#include <algorithm>
#include "Log.h"

ResourceManager& ResourceManager::GetInstance() {
    static ResourceManager instance;
    return instance;
}

ResourceManager::ResourceManager() {
}

// ---------------------------------------------------------------------
// Lógica Interna de Carga
// ---------------------------------------------------------------------

std::shared_ptr<Resource> ResourceManager::InternalLoad(const std::string& path, const std::string& typeName) {
    // 1. Verificar Cache
    if (m_resources.count(path)) {
        return m_resources.at(path);
    }

    // 2. Crear Recurso (Factory)
    std::shared_ptr<Resource> newResource = nullptr;
    if (typeName.find("Mesh") != std::string::npos) {
        newResource = std::make_shared<Mesh>();
    }
    // AQUÍ PUEDES AÑADIR TEXTURAS:
    // else if (typeName.find("Texture") != std::string::npos) { newResource = std::make_shared<Texture>(); }
    else {
        return nullptr;
    }

    // 3. Gestión de Library y Carga
    if (newResource) {
        // A) UUID
        VroomUUID uid = GetOrCreateMeta(path);

        // B) Ruta Library
        std::string libPath = "Assets/Library/" + std::to_string(uid);

        // C) Importación (Si no existe el binario, lo creamos)
        // Usamos std::filesystem para verificar si existe de forma fiable
        if (!std::filesystem::exists(libPath)) {
            std::cout << "[ResMan] Generando binario propio en Library para: " << path << std::endl;
            SaveToLibrary(path, uid);
        }

        // D) Configurar y Cargar
        newResource->SetUID(uid);
        newResource->SetAssetsPath(path);
        newResource->SetLibraryPath(libPath);

        // Al llamar a Load(), el Mesh leerá el archivo binario que acabamos de crear en SaveToLibrary
        newResource->Load();

        m_resources[path] = newResource;
    }

    return newResource;
}

// ---------------------------------------------------------------------
// IMPORTACIÓN Y GUARDADO (ESCRITURA BINARIA)
// ---------------------------------------------------------------------

void ResourceManager::SaveToLibrary(const std::string& assetPath, VroomUUID uid) {
    std::string libPath = "Assets/Library/" + std::to_string(uid);

    // Asegurar que el directorio existe
    std::filesystem::path p(libPath);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }

    std::string extension = assetPath.substr(assetPath.find_last_of(".") + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    if (extension == "fbx" || extension == "obj") {
        ImportMesh(assetPath, libPath);
    }
    else if (extension == "png" || extension == "jpg" || extension == "tga" || extension == "jpeg") {
        ImportTexture(assetPath, libPath);
    }
    else {
        Application::GetInstance().fileSystem->Copy(assetPath, libPath);
    }
}

void ResourceManager::ImportMesh(const std::string& assetPath, const std::string& libPath) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(assetPath, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || !scene->mRootNode || scene->mNumMeshes == 0) {
        std::cerr << "[Error] Assimp no pudo cargar: " << assetPath << std::endl;
        return;
    }

    aiMesh* mesh = scene->mMeshes[0]; // Simplificación: Solo la primera malla

    std::ofstream file(libPath, std::ios::binary);
    if (file.is_open()) {
        // 1. HEADER
        uint32_t numVertices = mesh->mNumVertices;
        uint32_t numIndices = 0;
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) numIndices += mesh->mFaces[i].mNumIndices;

        file.write((char*)&numVertices, sizeof(uint32_t));
        file.write((char*)&numIndices, sizeof(uint32_t));

        // 2. DATA - Vertices
        for (unsigned int i = 0; i < numVertices; i++) {
            float pos[3] = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
            file.write((char*)pos, sizeof(float) * 3);

            float norm[3] = { 0,0,0 };
            if (mesh->HasNormals()) {
                norm[0] = mesh->mNormals[i].x; norm[1] = mesh->mNormals[i].y; norm[2] = mesh->mNormals[i].z;
            }
            file.write((char*)norm, sizeof(float) * 3);

            float tex[2] = { 0,0 };
            if (mesh->HasTextureCoords(0)) {
                tex[0] = mesh->mTextureCoords[0][i].x; tex[1] = mesh->mTextureCoords[0][i].y;
            }
            file.write((char*)tex, sizeof(float) * 2);
        }

        // 3. DATA - Indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                uint32_t idx = face.mIndices[j];
                file.write((char*)&idx, sizeof(uint32_t));
            }
        }
        file.close();
        std::cout << "[Import OK] Mesh convertida a binario: " << libPath << std::endl;
    }
    else {
        std::cerr << "[Error] No se pudo escribir en Library: " << libPath << std::endl;
    }
}

void ResourceManager::ImportTexture(const std::string& assetPath, const std::string& libPath) {
    int width, height, channels;
    unsigned char* data = stbi_load(assetPath.c_str(), &width, &height, &channels, 0);

    if (data) {
        std::ofstream file(libPath, std::ios::binary);
        if (file.is_open()) {
            file.write((char*)&width, sizeof(int));
            file.write((char*)&height, sizeof(int));
            file.write((char*)&channels, sizeof(int));
            file.write((char*)data, width * height * channels);
            file.close();
            std::cout << "[Import OK] Textura convertida a binario: " << libPath << std::endl;
        }
        stbi_image_free(data);
    }
}

// ---------------------------------------------------------------------
// GESTIÓN DE META-DATA Y LIMPIEZA
// ---------------------------------------------------------------------

void ResourceManager::CleanUp() {
    m_resources.clear();
}

ResourceManager::~ResourceManager() {
    CleanUp();
}

// Asegúrate de poner este include arriba con los demás
#include "Log.h" 

void ResourceManager::ImportAssets() {
    LOG("--------------------------------------------------");
    LOG("[ResMan] INICIANDO IMPORTACION DE ASSETS...");

    // 1. OBTENER SISTEMA DE ARCHIVOS
    auto fileSys = Application::GetInstance().fileSystem;

    // 2. IMPRIMIR RUTA ACTUAL (Para ver si el Working Directory está mal)
    std::filesystem::path cwd = std::filesystem::current_path();
    LOG("[ResMan] Ruta de ejecucion (CWD): %s", cwd.string().c_str());

    // 3. BUSCAR ARCHIVOS
    std::vector<std::string> files = fileSys->GetAllFiles("Assets", true);

    if (files.empty()) {
        LOG("[ResMan] ERROR FATAL: No se han encontrado archivos en la carpeta 'Assets'.");
        LOG("[ResMan] -> Verifica el 'Working Directory' en las propiedades del proyecto.");
        return;
    }

    LOG("[ResMan] Se han encontrado %d archivos en Assets.", (int)files.size());

    // 4. PROCESAR
    int importados = 0;
    for (const std::string& path : files) {
        if (path.find(".meta") != std::string::npos) continue;

        VroomUUID uid = GetOrCreateMeta(path);
        std::string libraryPath = "Assets/Library/" + std::to_string(uid);

        // Si no existe, importamos
        if (!fileSys->Exists(libraryPath)) {
            LOG("[ResMan] Importando nuevo asset: %s", path.c_str());
            SaveToLibrary(path, uid);
            importados++;
        }
    }

    if (importados == 0) {
        LOG("[ResMan] Todos los assets ya estaban importados en Library.");
    }
    else {
        LOG("[ResMan] Proceso finalizado. Se importaron %d archivos.", importados);
    }
    LOG("--------------------------------------------------");
}

VroomUUID ResourceManager::GetOrCreateMeta(const std::string& path) {
    std::string metaPath = path + ".meta";
    auto fs = Application::GetInstance().fileSystem;

    if (fs->Exists(metaPath)) {
        std::ifstream file(metaPath);
        std::string line;
        if (file.is_open()) {
            while (std::getline(file, line)) {
                if (line.find("UID: ") != std::string::npos) {
                    try { return std::stoull(line.substr(5)); }
                    catch (...) { return 0; }
                }
            }
        }
    }

    VroomUUID newUID = UUIDGen::GenerateUUID();
    std::ofstream file(metaPath);
    if (file.is_open()) {
        file << "UID: " << newUID << "\n";
        file << "Time: " << fs->GetLastModTime(path) << "\n";
        file.close();
    }
    return newUID;
}