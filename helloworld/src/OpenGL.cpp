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
	/*shaderProgram = 3;*/
	glContext = NULL;
}

// Destructor
OpenGL::~OpenGL()
{

}

bool OpenGL::Start() {

	//context = environment in which all OpenGL commands operate
	//we create the context by passing a framebuffer, AKA a block of pixels displayable on a surface

	int version = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress));

	// … check for errors
	if (version == 0) {
		LOG("Error loading the glad library");
		return false;
	}

	viewMat = Application::GetInstance().camera->viewMat;
	projectionMat = Application::GetInstance().camera->projectionMat;

	/*stbi_set_flip_vertically_on_load(true);*/



	texCoordsShader = new Shader("TexCoordsShader.vert", "TexCoordsShader.frag");

	/*normalShader = new Shader("")*/

	std::cout << "OpenGL initialized successfully" << std::endl;


	/*If you declare a uniform that isn't used anywhere in your GLSL code
	the compiler will silently remove the variable from the compiled version
	is the cause for several frustrating errors; keep this in mind!*/
	//3D transformation matrices  --> Vclip = Mprojection⋅Mview⋅Mmodel⋅Vlocal

	modelMat = glm::mat4(1.0f);
	modelMat = glm::rotate(modelMat, glm::radians(45.0f), glm::vec3(0.0f, -1.0f, 0.0f)); //transforms vertex coordinates into world coordinates.
	//^rotates on the x axis so it looks like laying on the floor
	/*modelMat = glm::scale(modelMat, glm::vec3(0.05, 0.05, 0.05));*/

	texCoordsShader->Use();
	uint modelMatLoc = glad_glGetUniformLocation(texCoordsShader->ID, "model");
	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

	viewMat = glm::mat4(1.0f);
	// translate scene in the reverse direction of moving direction
	viewMat = glm::translate(viewMat, glm::vec3(0.0f, -2.0f, -15.0f));

	//OpenGL = righthanded system --> move cam in  positive z-axis (= translate scene towards negative z-axis)
	texCoordsShader->Use();
	uint viewMatLoc = glad_glGetUniformLocation(texCoordsShader->ID, "view");
	glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(viewMat));


	//projection mat = perspective (FOV, aspectRatio, nearPlane, farPlane)
	int windowW, windowH;
	Application::GetInstance().window.get()->GetSize(windowW, windowH);

	projectionMat = glm::mat4(1.0f);
	projectionMat = glm::perspective(glm::radians(45.0f), (float)windowW / windowH, 0.1f, 100.0f);
	texCoordsShader->Use();
	uint projectionMatLoc = glad_glGetUniformLocation(texCoordsShader->ID, "projection");
	glUniformMatrix4fv(projectionMatLoc, 1, GL_FALSE, glm::value_ptr(projectionMat));

	glEnable(GL_DEPTH_TEST);

	texCoordsShader->Use();


	std::string modelPath = "../Assets/Models/Street/street2.fbx";
	

	ourModel = new Model(modelPath.c_str());
	modelObjects.push_back(ourModel);
	Application::GetInstance().guiManager.get()->AddGameObject(ourModel);
	Application::GetInstance().render.get()->AddModel(ourModel);

	Model* cameraModel = new Model();
	modelObjects.push_back(cameraModel);
	Application::GetInstance().render.get()->AddModel(cameraModel);

	// Obtenemos el GameObject raíz
	std::shared_ptr<GameObject> camGO = cameraModel->GetRootGameObject();
	camGO->SetName("Main Camera"); // Le ponemos nombre para encontrarla fácil

	// IMPORTANTE: Añadimos a la lista de objetos de la escena del GUI
	Application::GetInstance().guiManager.get()->sceneObjects.push_back(camGO);

	// Añadimos el componente de cámara
	camGO->AddComponent(ComponentType::CAMERA);

	// Configuramos su posición inicial (Transform)
	// Si no lo movemos, estará dentro del edificio (0,0,0)
	auto transform = std::dynamic_pointer_cast<TransformComponent>(camGO->GetComponent(ComponentType::TRANSFORM));
	if (transform)
	{
		// La colocamos un poco elevada y hacia atrás (Z positivo)
		transform->SetPosition(glm::vec3(0.0f, 3.0f, 10.0f));

		// Opcional: Rotarla un poco para que mire hacia abajo (Pitch negativo)
		// transform->SetRotation(glm::vec3(-15.0f, 0.0f, 0.0f));
	}

	viewMat = glm::mat4(1.0f);


	return true;
}

bool OpenGL::Update(float dt)
{
	// 1. Limpieza de buffer
	// glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Opcional
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 2. Comprobación de renderizado (backface culling, blending, etc.)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);

	// 3. ACTUALIZAR MATRICES DE CÁMARA (¡Importante hacerlo AL PRINCIPIO!)
	// -----------------------------------------------------------------------
	if (useGameCamera && gameCamera != nullptr) {
		// Parche de seguridad por si borraste la cámara
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
		// Modo Editor
		viewMat = Application::GetInstance().camera->viewMat;
		projectionMat = Application::GetInstance().camera->projectionMat;
	}
	// -----------------------------------------------------------------------

	// 4. DIBUJAR EL GRID (SUELO)
	// -----------------------------------------------------------------------
	glUseProgram(texCoordsShader->ID);

	// A. Mandamos las matrices de cámara actualizadas
	texCoordsShader->setMat4("view", viewMat);
	texCoordsShader->setMat4("projection", projectionMat);

	// B. ¡CORRECCIÓN DEL ERROR!: Forzamos la matriz del modelo a Identidad (0,0,0)
	// Así el suelo siempre estará en el centro del mundo, ignorando a la casa.
	glm::mat4 identity = glm::mat4(1.0f);
	texCoordsShader->setMat4("model", identity);

	// C. Configuración visual del grid
	glUniform1i(glGetUniformLocation(texCoordsShader->ID, "useLineColor"), true);
	glUniform4f(glGetUniformLocation(texCoordsShader->ID, "lineColor"), 0.5f, 0.5f, 0.5f, 1.0f); // Grisáceo

	Application::GetInstance().render.get()->DrawGrid();

	// D. Desactivar modo líneas
	glUniform1i(glGetUniformLocation(texCoordsShader->ID, "useLineColor"), false);
	// -----------------------------------------------------------------------


	// 5. DIBUJAR LOS MODELOS (CASA, CÁMARA, ETC.)
	// -----------------------------------------------------------------------
	// Aquí cada modelo ya manda su propia matriz "model" dentro de su función Draw,
	// así que sobrescribirán la identidad que pusimos para el grid.
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

	// Helper arrays for normals and texcoords
	const glm::vec3 normals[6] = {
		glm::vec3(0.0f, 0.0f, 1.0f),   // Front
		glm::vec3(0.0f, 0.0f, -1.0f),  // Back
		glm::vec3(1.0f, 0.0f, 0.0f),   // Right
		glm::vec3(-1.0f, 0.0f, 0.0f),  // Left
		glm::vec3(0.0f, 1.0f, 0.0f),   // Top
		glm::vec3(0.0f, -1.0f, 0.0f)   // Bottom
	};

	const glm::vec2 texCoords[4] = {
		glm::vec2(0.0f, 0.0f), // Bottom left
		glm::vec2(1.0f, 0.0f), // Bottom right
		glm::vec2(1.0f, 1.0f), // Top right
		glm::vec2(0.0f, 1.0f)  // Top left
	};

	// Face definitions: position and normal index
	const glm::vec3 facePositions[6][4] = {
		{v001, v101, v111, v011}, // Front (+Z)
		{v100, v000, v010, v110}, // Back (-Z)
		{v101, v100, v110, v111}, // Right (+X)
		{v000, v001, v011, v010}, // Left (-X)
		{v011, v111, v110, v010}, // Top (+Y)
		{v000, v100, v101, v001}  // Bottom (-Y)
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

	// Build indices (2 triangles per face)
	for (int i = 0; i < 6; i++) {
		int base = i * 4;
		_indices.push_back(base);
		_indices.push_back(base + 1);
		_indices.push_back(base + 2);

		_indices.push_back(base);
		_indices.push_back(base + 2);
		_indices.push_back(base + 3);
	}

	// Create mesh
	Mesh cubeMesh(_vertices, _indices, _textures);

	// Create model from mesh
	Model* cubeModel = new Model(cubeMesh);

	// Add to manager
	modelObjects.push_back(cubeModel);
	Application::GetInstance().guiManager.get()->AddGameObject(cubeModel);
	cubeModel->GetRootGameObject().get()->SetName("Cube");

	return cubeModel;

}

