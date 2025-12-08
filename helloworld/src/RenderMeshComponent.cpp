#include "RenderMeshComponent.h"
#include "TransformComponent.h"
#include "MaterialComponent.h"
#include "GameObject.h"
#include "Shader.h"
#include "Log.h"

RenderMeshComponent::RenderMeshComponent(std::shared_ptr<GameObject> owner)
    : Component(owner, ComponentType::MESH_RENDERER), mesh(nullptr) {
}

RenderMeshComponent::~RenderMeshComponent() {
    mesh = nullptr;
}

void RenderMeshComponent::Enable() {}
void RenderMeshComponent::Update() {}
void RenderMeshComponent::Disable() {}
void RenderMeshComponent::OnEditor() {
    // Aquí puedes añadir ImGui para ver el UID del mesh, etc.
}

void RenderMeshComponent::SetMesh(std::shared_ptr<ResourceMesh> newMesh) {
    mesh = newMesh;
    if (mesh) mesh->LoadInMemory(); // Aseguramos que esté cargado al asignar
}

void RenderMeshComponent::Render(Shader* shader) {
    if (!active || !shader) return;

    // Si no hay mesh o no está cargado en VRAM, intentamos cargarlo
    if (!mesh) return;
    if (!mesh->IsLoaded()) mesh->LoadInMemory();

    auto sharedOwner = owner.lock();
    if (!sharedOwner) return;

    auto transform = std::dynamic_pointer_cast<TransformComponent>(sharedOwner->GetComponent(ComponentType::TRANSFORM));
    if (!transform) return;

    // Material handling (Simplificado para el ejemplo)
    auto material = std::dynamic_pointer_cast<MaterialComponent>(sharedOwner->GetComponent(ComponentType::MATERIAL));

    // Set uniforms
    shader->Use();
    shader->SetMat4("model", transform->GetGlobalTransform());

    // Pintar usando el recurso
    mesh->BindAndDraw();
}