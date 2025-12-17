#include "FileSystem.h"
#include "Log.h"
#include <algorithm>

// Alias para escribir menos
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

bool FileSystem::Copy(const std::string& source, const std::string& destination) {
    if (!Exists(source)) return false;
    try {
        // fs::copy_options::overwrite_existing permite sobrescribir si ya existe
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