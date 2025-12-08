#include "GameObject.h"
#include "TransformComponent.h"
#include "RenderMeshComponent.h"
#include "MaterialComponent.h"
#include <algorithm>
#include "Log.h"

GameObject::GameObject(const std::string& name_)
    : name(name_), active(true) {
}

GameObject::~GameObject() {
    Components.clear();
    children.clear();
}

void GameObject::Update() {
    for (auto& comp : Components) {
        if (comp && comp->IsActive())
            comp->Update();
    }

    for (auto& child : children)
        if (child)
            child->Update();
}

std::shared_ptr<Component> GameObject::AddComponent(ComponentType type) {
    std::shared_ptr<Component> newComponent;

    switch (type) {
    case ComponentType::TRANSFORM:
        newComponent = std::make_shared<TransformComponent>(shared_from_this());
        LOG("Added TRANSFORM component to GameObject '%s'", name.c_str());
        break;
    case ComponentType::MESH_RENDERER:
        newComponent = std::make_shared<RenderMeshComponent>(shared_from_this());
        LOG("Added MESH_RENDERER component to GameObject '%s'", name.c_str());
        break;
    case ComponentType::MATERIAL:
        newComponent = std::make_shared<MaterialComponent>(shared_from_this());
        LOG("Added MATERIAL component to GameObject '%s'", name.c_str());
        break;
    default:
        LOG("Unknown ComponentType");
        return nullptr;
    }

    if (newComponent) {
        Components.push_back(newComponent);
    }

    return newComponent;
}

std::shared_ptr<Component> GameObject::GetComponent(ComponentType type) {
    for (auto& comp : Components) {
        if (comp->GetType() == type) {
            return comp;
        }
    }
    return nullptr;
}

void GameObject::RemoveComponent(ComponentType type) {
    auto it = std::remove_if(Components.begin(), Components.end(),
        [type](const std::shared_ptr<Component>& comp) {
            return comp->GetType() == type;
        });

    if (it != Components.end()) {
        Components.erase(it, Components.end());
        LOG("Removed component of type %d from GameObject '%s'", type, name.c_str());
    }
}

void GameObject::SetActive(bool isActive) {
    active = isActive;
    for (auto& child : children)
        if (child)
            child->SetActive(isActive);
}

void GameObject::SetParent(std::shared_ptr<GameObject> newParent) {
    // Remove from current parent
    if (auto currentParent = parent.lock()) {
        currentParent->RemoveChild(shared_from_this());
    }

    parent = newParent;

    if (newParent) {
        newParent->AddChild(shared_from_this());
    }
}

std::shared_ptr<GameObject> GameObject::GetParent() const {
    return parent.lock();
}

const std::vector<std::shared_ptr<GameObject>>& GameObject::GetChildren() const {
    return children;
}

void GameObject::AddChild(std::shared_ptr<GameObject> child) {
    if (!child) return;

    // Prevent duplicates
    if (std::find(children.begin(), children.end(), child) != children.end())
        return;

    children.push_back(child);
    child->parent = shared_from_this(); // Direct assignment since friend/access allows or setter usage
}

void GameObject::RemoveChild(std::shared_ptr<GameObject> child) {
    if (!child) return;

    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        children.erase(it);
        child->parent.reset();
    }
}