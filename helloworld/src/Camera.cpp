#pragma once
#include "Application.h"
#include "Camera.h"
#include "Input.h"
#include "Window.h"
#include "GUIManager.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include <glm/gtx/string_cast.hpp>
#include "RenderMeshComponent.h"
#include "Mesh.h"
#include <limits>


Camera::Camera() : Module()
{
	name = "camera";
	// Initialize defaults here
	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	targetPos = glm::vec3(0.0f, 0.0f, 0.0f);
	yaw = -90.0f;
	pitch = 0.0f;
	fov = 45.0f;
	distance = glm::length(cameraPos - targetPos);

	firstMouse = true;
	lastX = 0.0f;
	lastY = 0.0f;
	xpos = 0.0f;
	ypos = 0.0f;

	fov = 45.0f;
	nearPlane = 0.1f;
	farPlane = 5000.0f;
}

Camera::~Camera()
{
}

bool Camera::Start()
{
	// Inicialización de matrices
	int windowW, windowH;
	Application::GetInstance().window->GetSize(windowW, windowH);

	UpdateCameraVectors();

	return true;
}

glm::vec3 Camera::ScreenPointToRay(float mouseX, float mouseY, int screenW, int screenH) const {

	// 1. Coordenadas normalizadas del dispositivo (NDC)
	float x = (2.0f * mouseX) / screenW - 1.0f;
	float y = 1.0f - (2.0f * mouseY) / screenH; // Invertir Y (SDL usa Y-abajo, OpenGL usa Y-arriba)

	// Usamos Z = -1.0 (cerca)
	glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

	// 2. Espacio de la Cámara (View Space)
	glm::mat4 invProjection = glm::inverse(GetProjectionMatrix());
	glm::vec4 rayEye = invProjection * rayClip;

	// Forzamos Z = -1.0 y W = 0.0 (es un vector de dirección)
	rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

	// 3. Espacio del Mundo (World Space)
	glm::mat4 invView = glm::inverse(GetViewMatrix());
	glm::vec4 rayWorld = invView * rayEye;

	// Normalizamos la dirección final
	return glm::normalize(glm::vec3(rayWorld));
}

bool Camera::Update(float dt)
{
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
	if (std::abs(wheelDelta) > 10000.0f)
		wheelDelta = 0.0f;

	if (wheelDelta != 0.0f)
	{
		ProcessScrollZoom(wheelDelta, true);
		if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT)
		{
			cameraPos = targetPos - cameraFront * distance;
		}
	}
	Application::GetInstance().input.get()->SetMouseWheelDeltaY(0);

	FocusObject(false);
	UpdateCameraVectors();

	int windowW, windowH;
	Application::GetInstance().window->GetSize(windowW, windowH);
	RecalculateMatrices(windowW, windowH);

	return true;
}

// Implementaciones de las funciones helper

void Camera::ProcessMouseRotation(float xoffset, float yoffset, float sensitivity)
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

void Camera::UpdateCameraVectors()
{
	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

void Camera::ProcessKeyboardMovement(float actualSpeed)
{
	glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

	if (Application::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
		cameraPos += actualSpeed * cameraFront;
	if (Application::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
		cameraPos -= actualSpeed * cameraFront;

	if (Application::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
		cameraPos += cameraRight * actualSpeed;
	if (Application::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
		cameraPos -= cameraRight * actualSpeed;

	if (Application::GetInstance().input->GetKey(SDL_SCANCODE_E) == KEY_REPEAT)
		cameraPos += worldUp * actualSpeed;
	if (Application::GetInstance().input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
		cameraPos -= worldUp * actualSpeed;
}

void Camera::ProcessPan(float xoffset, float yoffset)
{
	float panSpeed = 0.01f * (distance / 2);

	glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
	glm::vec3 cameraUpVector = glm::normalize(glm::cross(cameraRight, cameraFront));

	targetPos -= cameraRight * xoffset * panSpeed;
	targetPos += cameraUpVector * yoffset * panSpeed;
	cameraPos = targetPos - cameraFront * distance;
}

void Camera::ProcessScrollZoom(float delta, bool isMouseScroll)
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

void Camera::FocusObject(bool firstTime)
{
	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_F) == KEY_DOWN || firstTime)
	{
		std::shared_ptr<GameObject> selectedObj;
		auto& sceneObjs = Application::GetInstance().guiManager->sceneObjects;

		if (firstTime && !sceneObjs.empty())
			selectedObj = sceneObjs[0];
		else
			selectedObj = Application::GetInstance().guiManager->selectedObject;

		if (selectedObj)
		{
			fov = 60.0f;

			auto transform = std::dynamic_pointer_cast<TransformComponent>(
				selectedObj->GetComponent(ComponentType::TRANSFORM)
			);

			auto meshRenderer = std::dynamic_pointer_cast<RenderMeshComponent>(
				selectedObj->GetComponent(ComponentType::MESH_RENDERER)
			);

			glm::vec3 finalTarget = glm::vec3(0.0f);
			float finalDistance = 5.0f;

			if (transform && meshRenderer && meshRenderer->GetMesh() && !meshRenderer->GetMesh()->vertices.empty())
			{
				auto mesh = meshRenderer->GetMesh();

				glm::vec3 minAABB(std::numeric_limits<float>::max());
				glm::vec3 maxAABB(std::numeric_limits<float>::lowest());

				for (const auto& v : mesh->vertices) {
					minAABB = glm::min(minAABB, v.Position);
					maxAABB = glm::max(maxAABB, v.Position);
				}

				glm::mat4 globalModel = transform->GetGlobalTransform();
				glm::vec3 corners[8] = {
					{minAABB.x, minAABB.y, minAABB.z}, {minAABB.x, minAABB.y, maxAABB.z},
					{minAABB.x, maxAABB.y, minAABB.z}, {minAABB.x, maxAABB.y, maxAABB.z},
					{maxAABB.x, minAABB.y, minAABB.z}, {maxAABB.x, minAABB.y, maxAABB.z},
					{maxAABB.x, maxAABB.y, minAABB.z}, {maxAABB.x, maxAABB.y, maxAABB.z}
				};

				glm::vec3 worldMin(std::numeric_limits<float>::max());
				glm::vec3 worldMax(std::numeric_limits<float>::lowest());

				for (int i = 0; i < 8; i++) {
					glm::vec4 worldPt = globalModel * glm::vec4(corners[i], 1.0f);
					worldMin = glm::min(worldMin, glm::vec3(worldPt));
					worldMax = glm::max(worldMax, glm::vec3(worldPt));
				}

				finalTarget = (worldMin + worldMax) * 0.5f;
				float objectSize = glm::length(worldMax - worldMin);
				finalDistance = objectSize * 2.0f;
			}
			else if (transform)
			{
				finalTarget = transform->GetWorldPosition();
				finalDistance = 5.0f;
			}

			if (finalDistance < 2.0f) finalDistance = 2.0f;

			targetPos = finalTarget;
			distance = finalDistance;

			cameraPos = targetPos - cameraFront * distance;
		}
	}
}

void Camera::RecalculateMatrices(int windowW, int windowH)
{
	float aspectRatio = (float)windowW / (float)windowH;
	projectionMat = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
	viewMat = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}