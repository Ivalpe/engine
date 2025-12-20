#include "OpenGL.h"
#include <iostream>
#include "Log.h"
#include "Application.h"
#include "Window.h"
#include "Render.h"
#include "Textures.h"
#include "stb_image.h"
#include "Model.h"
#include "Input.h"
#include "Camera.h"
#include "GUIManager.h"
#include "CameraComponent.h"
#include "TransformComponent.h"
#include "GameObject.h"

OpenGL::OpenGL() : Module()
{
	std::cout << "OpenGL Constructor" << std::endl;
	VAO = 0;
	VBO = 1;
	EBO = 2;
	glContext = NULL;
}

OpenGL::~OpenGL()
{
}

bool OpenGL::Start() {

	int version = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress));

	if (version == 0) {
		LOG("Error loading the glad library");
		return false;
	}

	viewMat = Application::GetInstance().camera->viewMat;
	projectionMat = Application::GetInstance().camera->projectionMat;

	// AQUI ESTA EL CAMBIO DE LA RUTA RELATIVA
	texCoordsShader = new Shader("Assets/Shaders/TexCoordsShader.vert", "Assets/Shaders/TexCoordsShader.frag");

	std::cout << "OpenGL initialized successfully" << std::endl;

	modelMat = glm::mat4(1.0f);
	modelMat = glm::rotate(modelMat, glm::radians(45.0f), glm::vec3(0.0f, -1.0f, 0.0f));

	texCoordsShader->Use();
	uint modelMatLoc = glad_glGetUniformLocation(texCoordsShader->ID, "model");
	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

	viewMat = glm::mat4(1.0f);
	viewMat = glm::translate(viewMat, glm::vec3(0.0f, -2.0f, -15.0f));

	texCoordsShader->Use();
	uint viewMatLoc = glad_glGetUniformLocation(texCoordsShader->ID, "view");
	glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(viewMat));

	int windowW, windowH;
	Application::GetInstance().window.get()->GetSize(windowW, windowH);

	projectionMat = glm::mat4(1.0f);
	projectionMat = glm::perspective(glm::radians(45.0f), (float)windowW / windowH, 0.1f, 100.0f);
	texCoordsShader->Use();
	uint projectionMatLoc = glad_glGetUniformLocation(texCoordsShader->ID, "projection");
	glUniformMatrix4fv(projectionMatLoc, 1, GL_FALSE, glm::value_ptr(projectionMat));

	glEnable(GL_DEPTH_TEST);

	texCoordsShader->Use();

	std::string modelPath = "Assets/Models/Street/street2.fbx";

	ourModel = new Model(modelPath.c_str());
	modelObjects.push_back(ourModel);
	Application::GetInstance().guiManager.get()->AddGameObject(ourModel);
	Application::GetInstance().render.get()->AddModel(ourModel);

	Model* cameraModel = new Model();
	modelObjects.push_back(cameraModel);
	Application::GetInstance().render.get()->AddModel(cameraModel);

	std::shared_ptr<GameObject> camGO = cameraModel->GetRootGameObject();
	camGO->SetName("Main Camera");

	Application::GetInstance().guiManager.get()->sceneObjects.push_back(camGO);

	camGO->AddComponent(ComponentType::CAMERA);

	auto transform = std::dynamic_pointer_cast<TransformComponent>(camGO->GetComponent(ComponentType::TRANSFORM));
	if (transform)
	{
		transform->SetPosition(glm::vec3(0.0f, 3.0f, 10.0f));
	}

	viewMat = glm::mat4(1.0f);

	return true;
}

bool OpenGL::Update(float dt)
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);

	if (useGameCamera && gameCamera != nullptr) {
		if (!gameCamera->GetOwner() || gameCamera->GetOwner()->IsMarkedForDestroy()) {
			useGameCamera = false;
			gameCamera = nullptr;
		}
		else {
			viewMat = gameCamera->GetViewMatrix();
			projectionMat = gameCamera->GetProjectionMatrix();
		}
	}

	if (!useGameCamera) {
		viewMat = Application::GetInstance().camera->viewMat;
		projectionMat = Application::GetInstance().camera->projectionMat;
	}

	glUseProgram(texCoordsShader->ID);

	texCoordsShader->setMat4("view", viewMat);
	texCoordsShader->setMat4("projection", projectionMat);

	glm::mat4 identity = glm::mat4(1.0f);
	texCoordsShader->setMat4("model", identity);

	glUniform1i(glGetUniformLocation(texCoordsShader->ID, "useLineColor"), true);
	glUniform4f(glGetUniformLocation(texCoordsShader->ID, "lineColor"), 0.5f, 0.5f, 0.5f, 1.0f);

	Application::GetInstance().render.get()->DrawGrid();

	glUniform1i(glGetUniformLocation(texCoordsShader->ID, "useLineColor"), false);

	for (int i = 0; i < Application::GetInstance().render.get()->modelsToDraw.size(); i++) {
		Application::GetInstance().render.get()->modelsToDraw[i]->Draw(*texCoordsShader);
	}

	return true;
}

bool OpenGL::CleanUp() {
	glDeleteVertexArrays(1, &VAO);
	return true;
}

Model* OpenGL::CreateCube() {
	const glm::vec3 v000(-0.5f, -0.5f, -0.5f);
	const glm::vec3 v001(-0.5f, -0.5f, 0.5f);
	const glm::vec3 v010(-0.5f, 0.5f, -0.5f);
	const glm::vec3 v011(-0.5f, 0.5f, 0.5f);
	const glm::vec3 v100(0.5f, -0.5f, -0.5f);
	const glm::vec3 v101(0.5f, -0.5f, 0.5f);
	const glm::vec3 v110(0.5f, 0.5f, -0.5f);
	const glm::vec3 v111(0.5f, 0.5f, 0.5f);

	std::vector<Vertex> _vertices;
	std::vector<unsigned int> _indices;
	std::vector<Texture> _textures;

	const glm::vec3 normals[6] = {
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f)
	};

	const glm::vec2 texCoords[4] = {
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(0.0f, 1.0f)
	};

	const glm::vec3 facePositions[6][4] = {
		{v001, v101, v111, v011},
		{v100, v000, v010, v110},
		{v101, v100, v110, v111},
		{v000, v001, v011, v010},
		{v011, v111, v110, v010},
		{v000, v100, v101, v001}
	};

	for (int face = 0; face < 6; face++) {
		for (int vert = 0; vert < 4; vert++) {
			Vertex vertex;
			vertex.Position = facePositions[face][vert];
			vertex.Normal = normals[face];
			vertex.texCoord = texCoords[vert];
			_vertices.push_back(vertex);
		}
	}

	for (int i = 0; i < 6; i++) {
		int base = i * 4;
		_indices.push_back(base);
		_indices.push_back(base + 1);
		_indices.push_back(base + 2);

		_indices.push_back(base);
		_indices.push_back(base + 2);
		_indices.push_back(base + 3);
	}

	Mesh cubeMesh(_vertices, _indices, _textures);
	Model* cubeModel = new Model(cubeMesh);

	modelObjects.push_back(cubeModel);
	Application::GetInstance().guiManager.get()->AddGameObject(cubeModel);
	cubeModel->GetRootGameObject().get()->SetName("Cube");

	return cubeModel;
}