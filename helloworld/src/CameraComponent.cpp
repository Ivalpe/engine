#include "CameraComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "Application.h" 
#include "Window.h"
#include <glm/gtc/matrix_transform.hpp>

CameraComponent::CameraComponent(std::shared_ptr<GameObject> owner) : Component(owner, ComponentType::CAMERA)
{
    fov = 60.0f;
    nearPlane = 0.1f;
    farPlane = 1000.0f;
    aspectRatio = 16.0f / 9.0f;
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::Update()
{
    int w, h;
    Application::GetInstance().window->GetSize(w, h);
    SetAspectRatio((float)w, (float)h);
}

void CameraComponent::SetAspectRatio(float width, float height)
{
    if (height > 0)
        aspectRatio = width / height;
}

glm::mat4 CameraComponent::GetViewMatrix() const
{
    auto ownerPtr = owner.lock();
    if (!ownerPtr) return glm::mat4(1.0f);

    auto transform = std::dynamic_pointer_cast<TransformComponent>(ownerPtr->GetComponent(ComponentType::TRANSFORM));

    if (!transform) return glm::mat4(1.0f);

    return glm::inverse(transform->GetGlobalTransform());
}

glm::mat4 CameraComponent::GetProjectionMatrix() const
{
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}