#pragma once
#include "Module.h"
#include "Shader.h"
#include <vector>
#include "Model.h"

class OpenGL : public Module {

public:
	OpenGL();
	~OpenGL();

	SDL_GLContext glContext;
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;

	int width, height, nChannels;
	glm::mat4 modelMat, viewMat, projectionMat;

	// Eliminado cubePositions si no se usa o usar std::vector
	// glm::vec3* cubePositions = nullptr; 

	Shader* texCoordsShader;
	Model* ourModel;
	std::vector<Model*> modelObjects;

	bool Start() override;
	bool Update(float dt) override;
	bool CleanUp() override;

	// ELIMINADO: CreateCube() ya no es compatible con el nuevo sistema de recursos
	// Model* CreateCube(); 
};