#pragma once
#include "Component.h"
#include "ResourceMesh.h" // Importante: Usamos el nuevo recurso
#include <memory>

class Shader;

class RenderMeshComponent : public Component {
public:
    RenderMeshComponent(std::shared_ptr<GameObject> owner);
    ~RenderMeshComponent() override;

    // Component interface
    void Enable() override;
    void Update() override;
    void Disable() override;
    void OnEditor() override;

    // Mesh management
    void SetMesh(std::shared_ptr<ResourceMesh> newMesh);
    std::shared_ptr<ResourceMesh> GetMesh() const { return mesh; }

    // Rendering
    void Render(Shader* shader);

    bool drawAABB = false;
    void ToggleAABB(bool state) { drawAABB = state; }

private:
    std::shared_ptr<ResourceMesh> mesh; // Ahora es ResourceMesh
    bool drawFaceNormals = false;
    bool drawVertNormals = false;
};