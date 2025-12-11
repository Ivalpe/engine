#pragma once
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Textures.h"
#include <vector>
#include <SDL3/SDL_opengl.h>
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/mesh.h"
#include <limits>
#include "Resource.h"


using namespace std;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 texCoord;
};

struct AABB {
    // Inicializar min con valores muy grandes y max con valores muy pequeños
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());
};

class Texture;


class Mesh : public Resource {
public:
    // mesh data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;

    vector<glm::vec3>    normals;

    AABB meshAABB;

    Mesh() : Resource(ResourceType::MESH, "EmptyMesh") {}

    // CORRECCIÓN: Solo la declaración, sin cuerpo ni lista de inicialización
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);

    void Load() override;

    ~Mesh();
    void CalculateNormals();
    void CalculateAABB();
    void DrawAABB(Shader& shader, const glm::mat4& modelMatrix, const glm::vec4& color);
    void Draw(Shader& shader);
    bool drawVertNormals = false;
    bool drawFaceNormals = false;


private:
    //  render data
    unsigned int VAO, VBO, EBO;

    void setupMesh();

};