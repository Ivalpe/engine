#include "MaterialComponent.h"
#include "GameObject.h"
#include "Application.h"
#include "ResourceTexture.h" // Asegurar include

MaterialComponent::MaterialComponent(std::shared_ptr<GameObject> owner)
    : Component(owner, ComponentType::MATERIAL),
    diffuseColor(1.0f, 1.0f, 1.0f, 1.0f),
    shininess(32.0f),
    metallic(0.0f),
    roughness(0.5f) {
}

MaterialComponent::~MaterialComponent() {
    // Punteros inteligentes se limpian solos
}

void MaterialComponent::Enable() {}
void MaterialComponent::Update() {}
void MaterialComponent::Disable() {}
void MaterialComponent::OnEditor() {
    // Aquí iría tu código de ImGui
}