#pragma once
#include "Component.h"
#include "Mesh.h"  
#include "MaterialComponent.h"

class Shader;  // Forward declaration


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
    void SetMesh(std::shared_ptr<Mesh> newMesh);
    std::shared_ptr<Mesh>  GetMesh() const { return mesh; }

    // Rendering
    void Render(Shader* shader);  

    //// Drawing properties
    //void DrawNormals();

    //void SetMaterial(std::shared_ptr<MaterialComponent> material);
    //std::shared_ptr<MaterialComponent> GetMaterial() const;

    //void SetTexture(std::shared_ptr<Texture> texture);

    //void SetColor(const glm::vec3& color);
    //glm::vec3 GetColor() const;


private:
    std::shared_ptr<Mesh> mesh;  // Pointer to mesh data (not owned by this component)
    bool drawFaceNormals;
    bool drawVertNormals;
};