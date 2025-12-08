#include "ModelImporter.h"
#include "Application.h"
#include "FileSystem.h"
#include "Log.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <fstream>
#include <filesystem>
#include <vector>

namespace ModelImporter {

    bool Import(const char* filePath, const char* destDir) {
        LOG("Importing Model: %s", filePath);

        // 1. Cargar con Assimp
        const aiScene* scene = aiImportFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

        if (!scene || !scene->mRootNode) {
            LOG("Error importing Assimp: %s", aiGetErrorString());
            return false;
        }

        // 2. Iterar Meshes del FBX
        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            aiMesh* aiMesh = scene->mMeshes[i];

            // 3. Convertir datos a formato lineal (float array)
            // Estructura: Pos(3) + Norm(3) + UV(2) = 8 floats por vértice
            std::vector<float> vertices;
            std::vector<unsigned int> indices;

            for (unsigned int v = 0; v < aiMesh->mNumVertices; v++) {
                // -- Posiciones --
                vertices.push_back(aiMesh->mVertices[v].x);
                vertices.push_back(aiMesh->mVertices[v].y);
                vertices.push_back(aiMesh->mVertices[v].z);

                // -- Normales --
                if (aiMesh->HasNormals()) {
                    vertices.push_back(aiMesh->mNormals[v].x);
                    vertices.push_back(aiMesh->mNormals[v].y);
                    vertices.push_back(aiMesh->mNormals[v].z);
                }
                else {
                    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
                }

                // -- UVs --
                if (aiMesh->HasTextureCoords(0)) {
                    vertices.push_back(aiMesh->mTextureCoords[0][v].x);
                    vertices.push_back(aiMesh->mTextureCoords[0][v].y);
                }
                else {
                    vertices.push_back(0.0f); vertices.push_back(0.0f);
                }
            }

            // -- Índices --
            for (unsigned int f = 0; f < aiMesh->mNumFaces; f++) {
                aiFace& face = aiMesh->mFaces[f];
                if (face.mNumIndices == 3) { // Solo triángulos
                    indices.push_back(face.mIndices[0]);
                    indices.push_back(face.mIndices[1]);
                    indices.push_back(face.mIndices[2]);
                }
            }

            // 4. GUARDAR A FORMATO PROPIO (.vroom)
            // Generamos un nombre único basado en el nombre del archivo y el índice de la malla
            std::string fileName = std::filesystem::path(filePath).stem().string();
            // Ejemplo: Library/Meshes/BakerHouse_mesh_0.vroom
            std::string libPath = std::string(destDir) + "/" + fileName + "_mesh_" + std::to_string(i) + ".vroom";

            SaveToLibrary(libPath.c_str(), vertices.size() / 8, indices.size(), vertices.data(), indices.data());

            LOG("Generated Library Mesh: %s", libPath.c_str());
        }

        aiReleaseImport(scene);
        return true;
    }

    void SaveToLibrary(const char* path, uint32_t numVerts, uint32_t numIndices, const float* vertices, const unsigned int* indices) {
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            LOG("Error opening file for write: %s", path);
            return;
        }

        // CABECERA SIMPLE
        file.write((char*)&numVerts, sizeof(uint32_t));
        file.write((char*)&numIndices, sizeof(uint32_t));

        // DATOS (8 floats por vertice)
        file.write((char*)vertices, sizeof(float) * numVerts * 8);
        file.write((char*)indices, sizeof(unsigned int) * numIndices);

        file.close();
    }

    bool Load(ResourceMesh* resource) {
        if (!resource || resource->libraryPath.empty()) return false;

        std::ifstream file(resource->libraryPath, std::ios::binary);
        if (!file.is_open()) {
            LOG("Error loading binary mesh: %s", resource->libraryPath.c_str());
            return false;
        }

        // 1. LEER CABECERA
        uint32_t numVerts = 0;
        uint32_t numIndices = 0;
        file.read((char*)&numVerts, sizeof(uint32_t));
        file.read((char*)&numIndices, sizeof(uint32_t));

        // 2. LEER DATA A UN BUFFER TEMPORAL
        float* vertices = new float[numVerts * 8];
        unsigned int* indices = new unsigned int[numIndices];

        file.read((char*)vertices, sizeof(float) * numVerts * 8);
        file.read((char*)indices, sizeof(unsigned int) * numIndices);
        file.close();

        // 3. SUBIR A GPU (VRAM)
        if (resource->VAO == 0) glGenVertexArrays(1, &resource->VAO);
        if (resource->VBO == 0) glGenBuffers(1, &resource->VBO);
        if (resource->EBO == 0) glGenBuffers(1, &resource->EBO);

        glBindVertexArray(resource->VAO);

        glBindBuffer(GL_ARRAY_BUFFER, resource->VBO);
        // 8 floats por vertice (3 pos + 3 norm + 2 uv)
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numVerts * 8, vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, resource->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numIndices, indices, GL_STATIC_DRAW);

        // -- Configurar Atributos --
        // Posición (0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

        // Normales (1)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

        // UVs (2)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

        glBindVertexArray(0);

        // 4. LIMPIAR RAM
        delete[] vertices;
        delete[] indices;

        resource->indexCount = numIndices;

        LOG("Loaded Mesh to VRAM. VAO: %d, Indices: %d", resource->VAO, numIndices);

        return true;
    }
}