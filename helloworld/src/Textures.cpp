#include "Textures.h"
#include "Application.h"
#include "Render.h"
#include "Log.h"
#include <string>
#include <algorithm>
#include <iostream>
#include "FileSystem.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

Texture::Texture() : Module()
{
    name = "textures";
    id = -1;
    mapType = "";
    path = "";
    texW = 0;
    texH = 0;
}

Texture::~Texture() {}
bool Texture::Awake() { return true; }
bool Texture::Start() { return true; }
bool Texture::CleanUp() { return true; }

uint Texture::TextureFromFile(const string directory, const char* filename) {

    std::string editedDirectory = directory;
    std::replace(editedDirectory.begin(), editedDirectory.end(), '\\', '/');

    // --- CORRECCIÓN FINAL ---
    // ANTES (MALO): Esta línea cortaba la carpeta "Street"
    // editedDirectory = editedDirectory.substr(0, editedDirectory.find_last_of("/") + 1); 

    // AHORA (BUENO): La hemos borrado. Usamos el directorio tal cual.

    std::string filePath;
    // Añadimos la barra solo si falta y el directorio no está vacío
    if (!editedDirectory.empty() && editedDirectory.back() != '/') {
        filePath = editedDirectory + '/' + filename;
    }
    else {
        filePath = editedDirectory + filename;
    }

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    int width, height, nChannels;

    // Cargar imagen
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nChannels, 0);

    if (data)
    {
        GLenum format;
        if (nChannels == 1) format = GL_RED;
        else if (nChannels == 3) format = GL_RGB;
        else if (nChannels == 4) format = GL_RGBA;
        else format = GL_RGB;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Configuración estándar
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        texW = width;
        texH = height;
        stbi_image_free(data);

        // Puedes descomentar esto para verificar que carga:
        // cout << "[Texture] EXITO: " << filePath << endl;
    }
    else
    {
        // Si falla, avisamos por consola pero NO ponemos texturas rosas.
        // Se verá negro/blanco si falla.
        cout << "[Texture] ERROR: No se pudo cargar: " << filePath << endl;
        cout << "          Razon: " << stbi_failure_reason() << endl;
        stbi_image_free(data);
    }

    path = filePath;
    return id;
}