#pragma once
#include "Module.h"
#include <string>
#include <vector>
#include <filesystem> // C++17 Standard

class FileSystem : public Module {
public:
    FileSystem();
    ~FileSystem();

    bool Awake() override; // Usaremos Awake para crear carpetas base si no existen
    bool CleanUp() override;

    //  Importación ---
    // Escanea Assets y sincroniza con Library
    void ImportAssetsToLibrary();
    // Procesa un archivo individual y genera su .meta si falta
    void ProcessAsset(const std::string& sourcePath);


    // --- Path Utilities ---
    bool Exists(const std::string& path) const;
    bool IsDirectory(const std::string& path) const;
    bool CreateDir(const std::string& path);

    // --- File Manipulation ---
    // Copia un archivo de assets a library (o viceversa)
    bool Copy(const std::string& source, const std::string& destination);
    // Borra archivos o carpetas
    bool Delete(const std::string& path);

    // --- Discovery & Monitoring ---
    // Devuelve todos los archivos en un directorio (recursivo o no)
    std::vector<std::string> GetAllFiles(const std::string& dir, bool recursive = true);

    // Devuelve todos los archivos filtrados por extensión (ej: ".fbx")
    std::vector<std::string> GetFilesWithExtension(const std::string& dir, const std::string& extension, bool recursive = true);

    // Vital para detectar cambios en Assets y re-importar
    long long GetLastModTime(const std::string& path);

    // --- Helpers de String ---
    // Normaliza barras '/' y '\\' para evitar errores
    std::string NormalizePath(const std::string& path);
    // Separa el nombre del archivo de la ruta (ej: "Assets/model.fbx" -> "model.fbx")
    std::string GetFileName(const std::string& path);
    // Separa solo el nombre sin extensión (ej: "model")
    std::string GetFileNameNoExtension(const std::string& path);
    // Obtiene la extensión en minúsculas (ej: ".fbx")
    std::string GetFileExtension(const std::string& path);


private:
    bool NeedsReimport(const std::string& source, const std::string& destination);
};