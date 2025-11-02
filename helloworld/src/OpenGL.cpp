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


	std::string modelPath = "../Assets/Models/BakerHouse/BakerHouse.fbx";
	

	ourModel = new Model(modelPath.c_str());
	modelObjects.push_back(ourModel);

	Application::GetInstance().render.get()->AddModel(*ourModel);


	/*Model defaultCube = CreateCube();
	Application::GetInstance().render.get()->AddModel(defaultCube);*/


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

	viewMat = Application::GetInstance().camera->viewMat;
	projectionMat = Application::GetInstance().camera->projectionMat;

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
	std::vector<glm::vec3> _normals;
	std::vector<glm::vec2> _texCoords;
	std::vector<Texture> _textures;


	// Frontal face (+Z)
	_vertices.push_back({ v001 }); // 0
	_vertices.push_back({ v101 }); // 1
	_vertices.push_back({ v111 }); // 2
	_vertices.push_back({ v011 }); // 3

	// Back face (-Z)
	_vertices.push_back({ v100 }); // 4
	_vertices.push_back({ v000 }); // 5
	_vertices.push_back({ v010 }); // 6
	_vertices.push_back({ v110 }); // 7

	// Right face (+X)
	_vertices.push_back({ v101 }); // 8
	_vertices.push_back({ v100 }); // 9
	_vertices.push_back({ v110 }); // 10
	_vertices.push_back({ v111 }); // 11

	// Left face (-X)
	_vertices.push_back({ v000 }); // 12
	_vertices.push_back({ v001 }); // 13
	_vertices.push_back({ v011 }); // 14
	_vertices.push_back({ v010 }); // 15

	// Top face (+Y)
	_vertices.push_back({ v011 }); // 16
	_vertices.push_back({ v111 }); // 17
	_vertices.push_back({ v110 }); // 18
	_vertices.push_back({ v010 }); // 19

	// Bottom face (-Y)
	_vertices.push_back({ v000 }); // 20
	_vertices.push_back({ v100 }); // 21
	_vertices.push_back({ v101 }); // 22
	_vertices.push_back({ v001 }); // 23

	// Normals for each vertex
	for (int i = 0; i < 4; i++) _normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));  // Cara frontal
	for (int i = 0; i < 4; i++) _normals.push_back(glm::vec3(0.0f, 0.0f, -1.0f)); // Cara trasera
	for (int i = 0; i < 4; i++) _normals.push_back(glm::vec3(1.0f, 0.0f, 0.0f));  // Cara derecha
	for (int i = 0; i < 4; i++) _normals.push_back(glm::vec3(-1.0f, 0.0f, 0.0f)); // Cara izquierda
	for (int i = 0; i < 4; i++) _normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));  // Cara superior
	for (int i = 0; i < 4; i++) _normals.push_back(glm::vec3(0.0f, -1.0f, 0.0f)); // Cara inferior

	// tex coords for each face
	for (int i = 0; i < 6; i++) {
		_texCoords.push_back(glm::vec2(0.0f, 0.0f)); // Bottom left corner
		_texCoords.push_back(glm::vec2(1.0f, 0.0f)); // Bottom right corner
		_texCoords.push_back(glm::vec2(1.0f, 1.0f)); // Top right corner
		_texCoords.push_back(glm::vec2(0.0f, 1.0f)); // Top left corner
	}



	// indices for each face (2 triangles per face)
	for (int i = 0; i < 6; i++) {
		int base = i * 4;
		_indices.push_back(base);     // 0
		_indices.push_back(base + 1); // 1
		_indices.push_back(base + 2); // 2
		
		_indices.push_back(base);     // 0
		_indices.push_back(base + 2); // 2
		_indices.push_back(base + 3); // 3
	}

	// Assign data to model

	Mesh* cubeMesh = new Mesh(_vertices, _indices, _textures);

	//we probably need a function to create a model without a path
	Model* cubeModel = new Model(*cubeMesh);

	//add model and game object
	modelObjects.push_back(cubeModel);
	Application::GetInstance().guiManager.get()->AddGameObject(cubeModel);

	/*Application::GetInstance().render.get()->AddModel(cubeMesh);*/
	return *cubeModel;

}
