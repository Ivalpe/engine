#pragma once
#include "Component.h"
#include "glm/glm.hpp"

class CameraComponent : public Component
{
public:
    CameraComponent(std::shared_ptr<GameObject> owner);
    virtual ~CameraComponent();

    void Update() override;

    
    float fov;           
    float nearPlane;     
    float farPlane;      
    float aspectRatio;   

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;

    void SetAspectRatio(float width, float height);
};