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
#include "GUIManager.h"

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


	std::string modelPath = "../Assets/Models/BakerHouse/BakerHouse.fbx";
	

	ourModel = new Model(modelPath.c_str());
	modelObjects.push_back(ourModel);

	Application::GetInstance().render.get()->AddModel(*ourModel);


	/*Model defaultCube = CreateCube();
	Application::GetInstance().render.get()->AddModel(defaultCube);*/

	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	targetPos = glm::vec3(0.0f, 0.0f, 0.0f);
	distance = glm::length(cameraPos - targetPos);


	viewMat = glm::mat4(1.0f);


	return true;
}

bool OpenGL::Update(float dt) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glClearColor(0.1f, 0.2f, 0.3f, 1.0f); // dark bluish background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	glDisable(GL_CULL_FACE); //if defined clockwise, will not render



	//grid

	glUseProgram(texCoordsShader->ID);

	//use shader's line color instead of texture
	glUniform1i(glGetUniformLocation(texCoordsShader->ID, "useLineColor"), true);

	glUniform4f(glGetUniformLocation(texCoordsShader->ID, "lineColor"), 1.0f, 1.0f, 1.0f, 0.5f); //white grid



	Application::GetInstance().render.get()->DrawGrid();

	// Restore to normal texture mode
	glUniform1i(glGetUniformLocation(texCoordsShader->ID, "useLineColor"), false);

	//camera controls
	float cameraSpeed;


	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
		cameraSpeed = 0.20f;
	else
		cameraSpeed = 0.05f;

	xpos = Application::GetInstance().input.get()->GetMousePosition().x;
	ypos = Application::GetInstance().input.get()->GetMousePosition().y;
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	//Right Click
	if (Application::GetInstance().input.get()->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_REPEAT &&
		Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_LALT) != KEY_REPEAT)
	{
		ProcessKeyboardMovement(cameraSpeed);
		ProcessMouseRotation(xoffset, yoffset, 0.1f);
		UpdateCameraVectors();

		targetPos = cameraPos + cameraFront * distance;
	}

	if (Application::GetInstance().input.get()->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_UP &&
		Application::GetInstance().input.get()->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP &&
		Application::GetInstance().input.get()->GetMouseButtonDown(SDL_BUTTON_MIDDLE) == KEY_UP)
		firstMouse = true;



	//Alt + mouse
	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT)
	{
		if (Application::GetInstance().input.get()->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) // Orbitar
		{
			ProcessMouseRotation(xoffset, yoffset, 0.3f);
		}
		else if (Application::GetInstance().input.get()->GetMouseButtonDown(SDL_BUTTON_MIDDLE) == KEY_REPEAT) // Pan
		{
			ProcessPan(xoffset, -yoffset);
		}
		else if (Application::GetInstance().input.get()->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_REPEAT) // Dolly
		{
			float combinedDelta = xoffset - yoffset;
			ProcessScrollZoom(combinedDelta, false);
			cameraPos = targetPos - cameraFront * distance;
		}
	}

	lastX = xpos;
	lastY = ypos;

	// Mouse Wheel
	float wheelDelta = Application::GetInstance().input.get()->GetMouseWheelDeltaY();
	if (wheelDelta != 0.0f)
	{
		ProcessScrollZoom(wheelDelta, true);
		if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT)
		{
			cameraPos = targetPos - cameraFront * distance;
		}
	}
	Application::GetInstance().input.get()->SetMouseWheelDeltaY(0);

	FocusObject();
	UpdateCameraVectors();

	float aspectRatio = (float)Application::GetInstance().window.get()->width / (float)Application::GetInstance().window.get()->height;
	projectionMat = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
	viewMat = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	texCoordsShader->Use();
	texCoordsShader->setMat4("model", modelMat);
	texCoordsShader->setMat4("view", viewMat);
	texCoordsShader->setMat4("projection", projectionMat);

	//draw all meshes
	

	for (int i = 0; i < Application::GetInstance().render.get()->modelsToDraw.size(); i++) {
		Application::GetInstance().render.get()->modelsToDraw[i].Draw(*texCoordsShader);
	}

	return true;

}


bool OpenGL::CleanUp() {
	glDeleteVertexArrays(1, &VAO);

	return true;
}

void OpenGL::ProcessMouseRotation(float xoffset, float yoffset, float sensitivity)
{
	if (firstMouse)
	{
		lastX = Application::GetInstance().input.get()->GetMousePosition().x;
		lastY = Application::GetInstance().input.get()->GetMousePosition().y;
		firstMouse = false;
		return;
	}

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;


	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;


	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT)
	{
		UpdateCameraVectors();
		cameraPos = targetPos - cameraFront * distance;
	}
}

void OpenGL::UpdateCameraVectors()
{
	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

void OpenGL::ProcessKeyboardMovement(float actualSpeed)
{
	glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
		cameraPos += actualSpeed * cameraFront;
	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
		cameraPos -= actualSpeed * cameraFront;

	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
		cameraPos += cameraRight * actualSpeed;
	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
		cameraPos -= cameraRight * actualSpeed;

	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_E) == KEY_REPEAT)
		cameraPos += worldUp * actualSpeed;
	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
		cameraPos -= worldUp * actualSpeed;
}

void OpenGL::ProcessPan(float xoffset, float yoffset)
{
	float panSpeed = 0.01f * distance;

	glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
	glm::vec3 cameraUpVector = glm::normalize(glm::cross(cameraRight, cameraFront));

	targetPos -= cameraRight * xoffset * panSpeed;
	targetPos += cameraUpVector * yoffset * panSpeed;
	cameraPos = targetPos - cameraFront * distance;
}

void OpenGL::ProcessDolly(float xoffset, float yoffset)
{
	float combinedOffset = xoffset - yoffset;
	float dollySpeed = 0.01f * distance;
	distance -= combinedOffset * dollySpeed;
	if (distance < 0.1f) distance = 0.1f;
}

void OpenGL::ProcessScrollZoom(float delta, bool isMouseScroll)
{
	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT || !isMouseScroll) {

		float dollyMultiplier = isMouseScroll ? 0.5f : (0.01f * distance);
		distance -= delta * dollyMultiplier;

		if (distance < 0.1f)
			distance = 0.1f;
	}
	else {
		float zoomSpeed = 5.0f;
		fov -= delta * zoomSpeed;
		if (fov < 1.0f) fov = 1.0f;
		if (fov > 90.0f) fov = 90.0f;
	}
}


Model OpenGL::CreateCube() {
	const glm::vec3 v000(-1.0f, -1.0f, -1.0f);
	const glm::vec3 v001(-1.0f, -1.0f, 1.0f);
	const glm::vec3 v010(-1.0f, 1.0f, -1.0f);
	const glm::vec3 v011(-1.0f, 1.0f, 1.0f);
	const glm::vec3 v100(1.0f, -1.0f, -1.0f);
	const glm::vec3 v101(1.0f, -1.0f, 1.0f);
	const glm::vec3 v110(1.0f, 1.0f, -1.0f);
	const glm::vec3 v111(1.0f, 1.0f, 1.0f);

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

	return *cubeModel;
}

void OpenGL::FocusObject() {
	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_F) == KEY_DOWN)
	{
		std::shared_ptr<GameObject> selectedObj = Application::GetInstance().guiManager->selectedObject;
		if (selectedObj)
		{
			auto transformComp = std::dynamic_pointer_cast<TransformComponent>(
				selectedObj->GetComponent(ComponentType::TRANSFORM)
			);

			if (transformComp)
			{
				glm::vec3 targetPosition = transformComp->GetWorldPosition();

				UpdateCameraVectors();
				const float focusDistance = 7.0f;
				const float heightOffset = 1.0f;

				cameraPos = targetPosition - cameraFront * focusDistance + glm::vec3(0, heightOffset, 0);

				targetPos = targetPosition;
				distance = glm::length(cameraPos - targetPos);
			}
		}
	}
}

