#include "OpenGL.h"
#include <iostream>
#include "Log.h"
#include "Application.h"
#include "Window.h"
#include "Render.h"
#include "ResourceTexture.h"
// #include "stb_image.h" // ELIMINADO: No se usa aquí, causa conflictos si no se define la implementación
#include "Model.h"
#include "Input.h"
#include "Camera.h"
#include "GUIManager.h"

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
	// Carga de OpenGL con GLAD
	int version = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress));

	if (version == 0) {
		LOG("Error loading the glad library");
		return false;
	}

	viewMat = Application::GetInstance().camera->viewMat;
	projectionMat = Application::GetInstance().camera->projectionMat;

	texCoordsShader = new Shader("TexCoordsShader.vert", "TexCoordsShader.frag");

	std::cout << "OpenGL initialized successfully" << std::endl;

	// Matrices iniciales
	modelMat = glm::mat4(1.0f);
	modelMat = glm::rotate(modelMat, glm::radians(45.0f), glm::vec3(0.0f, -1.0f, 0.0f));

	texCoordsShader->Use();
	uint modelMatLoc = glad_glGetUniformLocation(texCoordsShader->ID, "model");
	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

	viewMat = glm::mat4(1.0f);
	viewMat = glm::translate(viewMat, glm::vec3(0.0f, -2.0f, -15.0f));

	uint viewMatLoc = glad_glGetUniformLocation(texCoordsShader->ID, "view");
	glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(viewMat));

	int windowW, windowH;
	Application::GetInstance().window.get()->GetSize(windowW, windowH);

	projectionMat = glm::mat4(1.0f);
	projectionMat = glm::perspective(glm::radians(45.0f), (float)windowW / windowH, 0.1f, 100.0f);
	uint projectionMatLoc = glad_glGetUniformLocation(texCoordsShader->ID, "projection");
	glUniformMatrix4fv(projectionMatLoc, 1, GL_FALSE, glm::value_ptr(projectionMat));

	glEnable(GL_DEPTH_TEST);

	// Cargar modelo inicial (BakerHouse)
	std::string modelPath = "../Assets/Models/BakerHouse/BakerHouse.fbx"; // Asegúrate de que la ruta sea correcta

	// Usamos el nuevo constructor de Model que usa el Importer internamente
	ourModel = new Model(modelPath.c_str());
	modelObjects.push_back(ourModel);

	// Añadimos a los gestores
	// Nota: Model ya no es un GameObject, contiene GameObjects.
	// Añadimos el objeto raíz a la escena.
	if (ourModel->GetRootGameObject()) {
		Application::GetInstance().guiManager.get()->sceneObjects.push_back(ourModel->GetRootGameObject());
	}

	// Render ya no necesita AddModel explícito si recorre la escena, 
	// pero si tu Render itera 'modelObjects', está bien.
	Application::GetInstance().render.get()->AddModel(ourModel);

	viewMat = glm::mat4(1.0f);

	return true;
}

bool OpenGL::Update(float dt) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);

	// Grid
	glUseProgram(texCoordsShader->ID);
	glUniform1i(glGetUniformLocation(texCoordsShader->ID, "useLineColor"), true);
	glUniform4f(glGetUniformLocation(texCoordsShader->ID, "lineColor"), 1.0f, 1.0f, 1.0f, 0.5f);

	Application::GetInstance().render.get()->DrawGrid();

	glUniform1i(glGetUniformLocation(texCoordsShader->ID, "useLineColor"), false);

	viewMat = Application::GetInstance().camera->viewMat;
	projectionMat = Application::GetInstance().camera->projectionMat;

	texCoordsShader->Use();
	texCoordsShader->setMat4("model", modelMat);
	texCoordsShader->setMat4("view", viewMat);
	texCoordsShader->setMat4("projection", projectionMat);

	// Dibujar modelos
	// NOTA: Con el nuevo sistema de componentes, idealmente recorrerías la escena y llamarías a Render en cada componente.
	// Por compatibilidad temporal:
	for (int i = 0; i < Application::GetInstance().render.get()->modelsToDraw.size(); i++) {
		// Model::Draw está obsoleto si usamos componentes, pero lo mantenemos si lo tienes implementado
		Application::GetInstance().render.get()->modelsToDraw[i]->Draw(*texCoordsShader);
	}

	return true;
}

bool OpenGL::CleanUp() {
	glDeleteVertexArrays(1, &VAO);
	return true;
}

// ELIMINADO: CreateCube() implementation
// La generación procedural requiere adaptar ResourceMesh para aceptar datos sin fichero, 
// lo cual es complejo. Mejor importar un cubo.fbx si necesitas un cubo.