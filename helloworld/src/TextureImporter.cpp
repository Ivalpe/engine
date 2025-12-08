#include "TextureImporter.h"
#include "Log.h"
#include "Application.h"
#include "FileSystem.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <SDL3/SDL_opengl.h>
#include <filesystem>
#include <fstream>
#include <vector>

namespace TextureImporter {

    bool Import(const char* filePath, const char* destDir) {
        LOG("Importing Texture: %s", filePath);

        int width, height, nrChannels;
        // stbi_set_flip_vertically_on_load(true); // Opcional, depende de tu UV mapping
        unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);

        if (!data) {
            LOG("Failed to load texture: %s", filePath);
            return false;
        }

        // Generar nombre en Library
        std::string fileName = std::filesystem::path(filePath).stem().string();
        std::string libPath = std::string(destDir) + "/" + fileName + ".vroom_tex";

        // Guardar en formato propio
        SaveToLibrary(libPath.c_str(), width, height, nrChannels, data);

        stbi_image_free(data);
        LOG("Texture Imported to Library: %s", libPath.c_str());
        return true;
    }

    void SaveToLibrary(const char* path, int width, int height, int channels, const void* data) {
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) return;

        // Cabecera simple
        file.write((char*)&width, sizeof(int));
        file.write((char*)&height, sizeof(int));
        file.write((char*)&channels, sizeof(int));

        // Datos
        size_t dataSize = width * height * channels;
        file.write((char*)data, dataSize);

        file.close();
    }

    bool Load(ResourceTexture* resource) {
        if (!resource || resource->libraryPath.empty()) return false;

        std::ifstream file(resource->libraryPath, std::ios::binary);
        if (!file.is_open()) {
            LOG("Error loading binary texture: %s", resource->libraryPath.c_str());
            return false;
        }

        // Leer Cabecera
        int width, height, channels;
        file.read((char*)&width, sizeof(int));
        file.read((char*)&height, sizeof(int));
        file.read((char*)&channels, sizeof(int));

        resource->width = width;
        resource->height = height;

        // Leer Datos
        size_t dataSize = width * height * channels;
        std::vector<unsigned char> data(dataSize);
        file.read((char*)data.data(), dataSize);
        file.close();

        // Subir a GPU
        if (resource->textureID == 0) glGenTextures(1, &resource->textureID);
        glBindTexture(GL_TEXTURE_2D, resource->textureID);

        GLenum format = GL_RGB;
        if (channels == 1) format = GL_RED;
        else if (channels == 3) format = GL_RGB;
        else if (channels == 4) format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data.data());
        glGenerateMipmap(GL_TEXTURE_2D);

        // Parámetros
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        LOG("Loaded Texture to VRAM. ID: %d (%dx%d)", resource->textureID, width, height);
        return true;
    }
}