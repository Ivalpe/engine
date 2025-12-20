#include "FileSystem.h"
#include "Log.h"
#include <algorithm>
#include <fstream>


namespace fs = std::filesystem;

FileSystem::FileSystem() {
    name = "FileSystem";
}

FileSystem::~FileSystem() {
}

bool FileSystem::Awake() {
    LOG("FileSystem::Awake");

    // Asegurarnos de que las carpetas base existan
    if (!Exists("Assets")) CreateDir("Assets");
    if (!Exists("Assets/Library")) CreateDir("Assets/Library");

    ImportAssetsToLibrary();

    return true;
}

bool FileSystem::CleanUp() {
    return true;
}

bool FileSystem::Exists(const std::string& path) const {
    return fs::exists(path);
}

bool FileSystem::IsDirectory(const std::string& path) const {
    return fs::is_directory(path);
}

bool FileSystem::CreateDir(const std::string& path) {
    if (Exists(path)) return true;
    try {
        return fs::create_directories(path);
    }
    catch (fs::filesystem_error& e) {
        LOG("Error creating directory %s: %s", path.c_str(), e.what());
        return false;
    }
}

//bool FileSystem::Copy(const std::string& source, const std::string& destination) {
//    if (!Exists(source)) return false;
//    try {
//        // fs::copy_options::overwrite_existing permite sobrescribir si ya existe
//        fs::copy(source, destination, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
//        return true;
//    }
//    catch (fs::filesystem_error& e) {
//        LOG("Error copying from %s to %s: %s", source.c_str(), destination.c_str(), e.what());
//        return false;
//    }
//}

bool FileSystem::Copy(const std::string& source, const std::string& destination) {
    if (!Exists(source)) return false;
    try {
        // Si el destino es un archivo, usamos copy_file que es más específico
        if (!fs::is_directory(source)) {
            return fs::copy_file(source, destination, fs::copy_options::overwrite_existing);
        }
        
        // Si es directorio, usamos copy normal
        fs::copy(source, destination, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
        return true;
    }
    catch (fs::filesystem_error& e) {
        LOG("Error copying from %s to %s: %s", source.c_str(), destination.c_str(), e.what());
        return false;
    }
}


bool FileSystem::Delete(const std::string& path) {
    if (!Exists(path)) return false;
    try {
        // remove_all borra recursivamente (útil para carpetas)
        return fs::remove_all(path) > 0;
    }
    catch (fs::filesystem_error& e) {
        LOG("Error deleting %s: %s", path.c_str(), e.what());
        return false;
    }
}

std::vector<std::string> FileSystem::GetAllFiles(const std::string& dir, bool recursive) {
    std::vector<std::string> files;
    if (!Exists(dir)) return files;

    try {
        if (recursive) {
            for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                if (fs::is_regular_file(entry)) {
                    files.push_back(NormalizePath(entry.path().string()));
                }
            }
        }
        else {
            for (const auto& entry : fs::directory_iterator(dir)) {
                if (fs::is_regular_file(entry)) {
                    files.push_back(NormalizePath(entry.path().string()));
                }
            }
        }
    }
    catch (fs::filesystem_error& e) {
        LOG("Error iterating directory %s: %s", dir.c_str(), e.what());
    }
    return files;
}

std::vector<std::string> FileSystem::GetFilesWithExtension(const std::string& dir, const std::string& extension, bool recursive) {
    std::vector<std::string> allFiles = GetAllFiles(dir, recursive);
    std::vector<std::string> filteredFiles;

    std::string extLower = extension;
    // Asegurar que la extension a buscar este en minusculas para comparar
    std::transform(extLower.begin(), extLower.end(), extLower.begin(), ::tolower);

    for (const auto& file : allFiles) {
        if (GetFileExtension(file) == extLower) {
            filteredFiles.push_back(file);
        }
    }
    return filteredFiles;
}

long long FileSystem::GetLastModTime(const std::string& path) {
    if (!Exists(path)) return 0;
    try {
        auto time = fs::last_write_time(path);
        // Convertir a un entero simple (timestamp)
        return std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
    }
    catch (fs::filesystem_error& e) {
        LOG("Error getting mod time for %s: %s", path.c_str(), e.what());
        return 0;
    }
}

std::string FileSystem::NormalizePath(const std::string& path) {
    std::string cleanPath = path;
    std::replace(cleanPath.begin(), cleanPath.end(), '\\', '/');
    return cleanPath;
}

std::string FileSystem::GetFileName(const std::string& path) {
    return fs::path(path).filename().string();
}

std::string FileSystem::GetFileNameNoExtension(const std::string& path) {
    return fs::path(path).stem().string();
}

std::string FileSystem::GetFileExtension(const std::string& path) {
    std::string ext = fs::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

//void FileSystem::ImportAssetsToLibrary() {
//    LOG("FileSystem::Sincronizando Assets con Library...");
//
//    // Obtenemos todos los archivos de Assets
//    std::vector<std::string> allAssets = GetAllFiles("Assets", true);
//    LOG("Archivos encontrados en Assets: %d", (int)allAssets.size());
//
//    for (const std::string& assetPath : allAssets) {
//        // Ignorar archivos que ya están dentro de Library
//        if (assetPath.find("Assets/Library") != std::string::npos) continue;
//
//        // Ignorar los propios archivos .meta
//        if (GetFileExtension(assetPath) == ".meta") continue;
//
//        ProcessAsset(assetPath);
//    }
//}

void FileSystem::ImportAssetsToLibrary() {
   // std::string assetsPath = "Assets";

    // 1. Si no existe en la carpeta actual, probamos una carpeta atrás (../Assets)
    // Esto es común si el exe está en Build/ o Debug/
   // if (!Exists(assetsPath)) {}
    std::string assetsPath = "../Assets";


    // 2. Si sigue sin existir, probamos dos carpetas atrás (../../Assets)
    // Común en estructuras de Visual Studio (x64/Debug/)
    if (!Exists(assetsPath)) {
        assetsPath = "../../Assets";
    }

    if (!Exists(assetsPath)) {
        LOG("ERROR: No se pudo encontrar la carpeta Assets en ninguna ruta relativa.");
        return;
    }

    // 2. Imprime la ruta absoluta para que verifiques en Windows si es la correcta
    LOG("Carpeta Assets encontrada en: %s", fs::absolute(assetsPath).string().c_str());

   // LOG("Carpeta Assets detectada en: %s", assetsPath.c_str());

    // Ahora usamos esa ruta encontrada para buscar los archivos
    std::vector<std::string> allAssets = GetAllFiles(assetsPath, true);

    for (const std::string& assetPath : allAssets) {
        // Importante: No procesar lo que ya esté en Library
        if (assetPath.find("/Library/") != std::string::npos) continue;
        if (GetFileExtension(assetPath) == ".meta") continue;

        ProcessAsset(assetPath);
    }
}

void FileSystem::ProcessAsset(const std::string& sourcePath) {
    std::string cleanSource = NormalizePath(sourcePath);

    // Encontrar dónde empieza "Assets/" para ignorar los "../" previos
    size_t assetsPos = cleanSource.find("Assets/");
    if (assetsPos == std::string::npos) return;

    // relativePath será algo como "Textures/wood.png"
    std::string relativePath = cleanSource.substr(assetsPos + 7); // 7 es la longitud de "Assets/"

    // El destino SIEMPRE debe ser relativo al ejecutable actual
    std::string destination = "Assets/Library/" + relativePath + ".bin";
    destination = NormalizePath(destination);

    // Asegurar carpetas de destino
    std::string destFolder = fs::path(destination).parent_path().string();
    if (!Exists(destFolder)) CreateDir(destFolder);

	std::string fileName = GetFileName(cleanSource);
    // 5. Verificar si el archivo necesita ser re-importado
    if (NeedsReimport(cleanSource, destination)) {
        LOG("[FileSystem] Sincronizando Asset: %s", relativePath.c_str());

        // COPIAR: En el futuro aquí llamarás a funciones como ImportMesh() o ImportTexture()
        // Por ahora, copiamos el archivo original a la carpeta espejo en Library
        if (Copy(cleanSource, destination)) {

            // 6. GESTIÓN DE METADATOS: Crear .meta al lado del archivo original (exterior)
            std::string metaPath = cleanSource + ".meta";
            if (!Exists(metaPath)) {
                std::ofstream metaFile(metaPath);
                if (metaFile.is_open()) {
                    // Guardamos información básica en el meta
                    metaFile << "resource_name: " << fileName << "\n";
                    metaFile << "original_rel_path: " << relativePath << "\n";
                    metaFile << "import_version: 1.0\n";
                    metaFile.close();
                    LOG("[FileSystem] Generado meta: %s", metaPath.c_str());
                }
            }
        }
    }
}

bool FileSystem::NeedsReimport(const std::string& source, const std::string& destination) {
    if (!Exists(destination)) return true;

    try {
        // Usamos la comparación directa de la librería filesystem (más precisa que convertir a long long)
        auto sourceTime = fs::last_write_time(source);
        auto destTime = fs::last_write_time(destination);

        return sourceTime > destTime;
    }
    catch (fs::filesystem_error& e) {
        return true; // Ante la duda, re-importamos
    }
}