#include "Mesh.h"
#include "Shader.h"
#include "Model.h"
#include "Application.h"
#include "GuiManager.h"
#include "Render.h"
#include "Resource.h"
#include <fstream>

// CORRECCIÓN: Se añade ": Resource(...)" para inicializar la clase base
Mesh::Mesh(vector<Vertex> _vertices, vector<unsigned int> _indices, vector<Texture> _textures)
    : Resource(ResourceType::MESH, "Mesh")
{
    this->vertices = _vertices;
    this->indices = _indices;
    this->textures = _textures;

    drawVertNormals = false;
    drawFaceNormals = false;

    this->setupMesh();
    CalculateNormals();
    CalculateAABB();
}

Mesh::~Mesh() {

}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW); //send to OpenGL (GPU)

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW); //send to GPU

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    glBindVertexArray(0);




}

void Mesh::CalculateAABB() {
    // Reset min/max a valores extremos
    meshAABB.min = glm::vec3(std::numeric_limits<float>::max());
    meshAABB.max = glm::vec3(std::numeric_limits<float>::lowest());

    // Iterar sobre todos los vértices del mesh
    for (const auto& vertex : vertices) {
        // Actualizar min
        meshAABB.min.x = std::min(meshAABB.min.x, vertex.Position.x);
        meshAABB.min.y = std::min(meshAABB.min.y, vertex.Position.y);
        meshAABB.min.z = std::min(meshAABB.min.z, vertex.Position.z);

        // Actualizar max
        meshAABB.max.x = std::max(meshAABB.max.x, vertex.Position.x);
        meshAABB.max.y = std::max(meshAABB.max.y, vertex.Position.y);
        meshAABB.max.z = std::max(meshAABB.max.z, vertex.Position.z);
    }
}

void Mesh::DrawAABB(Shader& shader, const glm::mat4& modelMatrix, const glm::vec4& color) {

    // Cambiar a modo wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Configurar la matriz del modelo (Transformación de World Space)
    GLint modelLoc = glGetUniformLocation(shader.ID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Configurar color para visualización
    glUniform1i(glGetUniformLocation(shader.ID, "useLineColor"), true);
    glUniform4f(glGetUniformLocation(shader.ID, "lineColor"), color.r, color.g, color.b, color.a);
    glLineWidth(2.0f); // Puedes ajustar el ancho de línea

    glm::vec3 min = meshAABB.min;
    glm::vec3 max = meshAABB.max;

    // Definir los 8 vértices del AABB
    glm::vec3 v[8] = {
        glm::vec3(min.x, min.y, min.z), // 0
        glm::vec3(max.x, min.y, min.z), // 1
        glm::vec3(min.x, max.y, min.z), // 2
        glm::vec3(max.x, max.y, min.z), // 3
        glm::vec3(min.x, min.y, max.z), // 4
        glm::vec3(max.x, min.y, max.z), // 5
        glm::vec3(min.x, max.y, max.z), // 6
        glm::vec3(max.x, max.y, max.z)  // 7
    };

    // Dibujar las 12 aristas usando GL_LINES
    glBegin(GL_LINES);
    // Cara inferior
    glVertex3fv(glm::value_ptr(v[0])); glVertex3fv(glm::value_ptr(v[1]));
    glVertex3fv(glm::value_ptr(v[1])); glVertex3fv(glm::value_ptr(v[5]));
    glVertex3fv(glm::value_ptr(v[5])); glVertex3fv(glm::value_ptr(v[4]));
    glVertex3fv(glm::value_ptr(v[4])); glVertex3fv(glm::value_ptr(v[0]));

    // Cara superior
    glVertex3fv(glm::value_ptr(v[2])); glVertex3fv(glm::value_ptr(v[3]));
    glVertex3fv(glm::value_ptr(v[3])); glVertex3fv(glm::value_ptr(v[7]));
    glVertex3fv(glm::value_ptr(v[7])); glVertex3fv(glm::value_ptr(v[6]));
    glVertex3fv(glm::value_ptr(v[6])); glVertex3fv(glm::value_ptr(v[2]));

    // Conexiones verticales
    glVertex3fv(glm::value_ptr(v[0])); glVertex3fv(glm::value_ptr(v[2]));
    glVertex3fv(glm::value_ptr(v[1])); glVertex3fv(glm::value_ptr(v[3]));
    glVertex3fv(glm::value_ptr(v[5])); glVertex3fv(glm::value_ptr(v[7]));
    glVertex3fv(glm::value_ptr(v[4])); glVertex3fv(glm::value_ptr(v[6]));
    glEnd();

    // Resetear el color de línea y el modo de polígono a relleno
    glUniform1i(glGetUniformLocation(shader.ID, "useLineColor"), false);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Mesh::Draw(Shader& shader) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;



    for (unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0); // activate proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        string number;
        string name = textures[i].mapType;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);

        shader.setInt(("material." + name + number).c_str(), i);


        glBindTexture(GL_TEXTURE_2D, textures[i].id);

    }



    if (drawFaceNormals) {


        glUniform1i(glGetUniformLocation(shader.ID, "useLineColor"), true);
        glUniform4f(glGetUniformLocation(shader.ID, "lineColor"), 0.0f, 1.0f, 0.0f, 1.0f); //green for vertex


        glBegin(GL_LINES);


        for (int i = 0; i < indices.size(); i += 3) {
            glm::vec3 start = vertices[indices[i]].Position;
            glm::vec3 end = start + normals[indices[i]] * 0.2f;
            glVertex3fv(glm::value_ptr(start));
            glVertex3fv(glm::value_ptr(end));
        }

        glEnd();
        glUniform1i(glGetUniformLocation(shader.ID, "useLineColor"), false);
    }

    if (drawVertNormals) {


        glUniform1i(glGetUniformLocation(shader.ID, "useLineColor"), true);
        glUniform4f(glGetUniformLocation(shader.ID, "lineColor"), 0.0f, 0.9f, 1.0f, 1.0f); //blue for face

        glBegin(GL_LINES);


        for (int i = 0; i < vertices.size(); i += 3) {
            glm::vec3 v0 = vertices[indices[i]].Position;
            glm::vec3 v1 = vertices[indices[i + 1]].Position;
            glm::vec3 v2 = vertices[indices[i + 2]].Position;


            glm::vec3 normalDir = glm::normalize(glm::cross(v1 - v0, v2 - v0));
            glm::vec3 center = (v0 + v1 + v2) / 3.0f;
            glm::vec3 end = center + normalDir * 0.2f;


            glVertex3fv(glm::value_ptr(center));
            glVertex3fv(glm::value_ptr(end));
        }
        glEnd();
        glUniform1i(glGetUniformLocation(shader.ID, "useLineColor"), false);
    }


    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0); //reset texture units for next draw call!
}

void Mesh::CalculateNormals() {
    normals.resize(vertices.size(), glm::vec3(0.0f));


    for (int i = 0; i < indices.size(); i += 3) {
        glm::vec3 v0 = vertices[indices[i]].Position;
        glm::vec3 v1 = vertices[indices[i + 1]].Position;
        glm::vec3 v2 = vertices[indices[i + 2]].Position;

        //With just vertices[i] instead of vertices[indices[i]], you’d be assuming that every 3 consecutive vertices form a triangle.
        //However, that's not always the case, as most meshes reuse vertices between faces.

        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        //compute cross product with v0->v1, v0->v2 
        //both from the same point (v0), because the resulting perpendicular vector has to sit on a common vertex

        //add normal vector on top of each vertex
        normals[indices[i]] += normal;
        normals[indices[i + 1]] += normal;
        normals[indices[i + 2]] += normal;

    }

    for (glm::vec3 normal : normals)
    {
        //since we added the normals to the indices, they aren't normalized anymore, so we do it again
        normal = glm::normalize(normal);
    }

}

void Mesh::Load() {
    // Usamos la ruta de Library que ResMan nos ha asignado
    std::string path = this->GetLibraryPath();
    if (path.empty()) return;

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "ERROR: No se pudo abrir el binario: " << path << std::endl;
        return;
    }

    // 1. Leer Header
    uint32_t numVertices = 0;
    uint32_t numIndices = 0;
    file.read((char*)&numVertices, sizeof(uint32_t));
    file.read((char*)&numIndices, sizeof(uint32_t));

    vertices.resize(numVertices);
    indices.resize(numIndices);

    // 2. Leer Datos
    for (uint32_t i = 0; i < numVertices; i++) {
        file.read((char*)&vertices[i].Position, sizeof(float) * 3);
        file.read((char*)&vertices[i].Normal, sizeof(float) * 3);
        file.read((char*)&vertices[i].texCoord, sizeof(float) * 2);
    }
    file.read((char*)&indices[0], numIndices * sizeof(unsigned int));

    file.close();

    // 3. Regenerar buffers de OpenGL
    setupMesh();
    CalculateNormals();
    CalculateAABB();

    std::cout << "Mesh cargada desde binario propio (.vroom)! Vértices: " << numVertices << std::endl;
}