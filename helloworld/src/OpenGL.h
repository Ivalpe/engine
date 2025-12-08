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
	/*unsigned int shaderProgram;*/
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;

	
	int width, height, nChannels;
	glm::mat4 modelMat, viewMat, projectionMat;
	glm::vec3* cubePositions = nullptr;

	Shader* texCoordsShader;
	Model* ourModel;
	std::vector<Model*> modelObjects;

	bool Start() override;
	bool Update(float dt) override;
	bool CleanUp() override;

	Model* CreateCube();
};
