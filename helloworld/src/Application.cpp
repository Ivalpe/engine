#include "Application.h"
#include <iostream>
#include "Log.h"

#include "Window.h"
#include "Input.h"
#include "Render.h"
#include "OpenGL.h"
#include "FileSystem.h"
#include "Textures.h"

#include "GUIManager.h"
#include "RenderMeshComponent.h"
#include "TransformComponent.h"
#include "Camera.h"
#include "Mesh.h"
#include <limits>
#include <algorithm>



// Constructor
Application::Application() {

    LOG("Constructor Application::Application");

    // Modules
    window = std::make_shared<Window>();
    guiManager = std::make_shared<GUIManager>();
    input = std::make_shared<Input>();
    render = std::make_shared<Render>();
    openGL = std::make_shared<OpenGL>();
    fileSystem = std::make_shared<FileSystem>();
    textures = std::make_shared<Texture>();
    camera = std::make_shared<Camera>();

    // Ordered for awake / Start / Update
    // Reverse order of CleanUp
    AddModule(std::static_pointer_cast<Module>(window));
    AddModule(std::static_pointer_cast<Module>(guiManager));
    AddModule(std::static_pointer_cast<Module>(input));
    AddModule(std::static_pointer_cast<Module>(textures));
    AddModule(std::static_pointer_cast<Module>(camera));


    // Render last 
    AddModule(std::static_pointer_cast<Module>(openGL));
    AddModule(std::static_pointer_cast<Module>(fileSystem));
    AddModule(std::static_pointer_cast<Module>(render));

}

// Static method to get the instance of the Application class, following the singletn pattern
Application& Application::GetInstance() {
    static Application instance; // Guaranteed to be destroyed and instantiated on first use
    return instance;
}

void Application::AddModule(std::shared_ptr<Module> module) {
    module->Init();
    moduleList.push_back(module);
}

// Called before render is available
bool Application::Awake() {

    LOG("Application::Awake");

    //Iterates the module list and calls Awake on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->Awake();
        if (!result) {
            break;
        }
    }

    return result;
}

// Called before the first frame
bool Application::Start() {
    LOG("Application::Start");

    //Iterates the module list and calls Start on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->Start();
        if (!result) {
            break;
        }
    }

    return result;
}

// Called each loop iteration
bool Application::Update() {

    if (requestExit) return false;
    
    bool ret = true;
    PrepareUpdate();

    if (input->GetWindowEvent(WE_QUIT) == true)
        ret = false;

    if (ret == true)
        ret = PreUpdate();

    if (ret == true)
        ret = DoUpdate();

    if (ret == true)
        ret = PostUpdate();

    FinishUpdate();
    return ret;
}

// Called before quitting
bool Application::CleanUp() {
    LOG("Application::CleanUp");

    //Iterates the module list and calls CleanUp on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->CleanUp();
        if (!result) {
            break;
        }
    }

    return result;
}

// ---------------------------------------------
void Application::PrepareUpdate()
{
}

// ---------------------------------------------
void Application::FinishUpdate()
{
    if (openGL && openGL->ourModel) {
        openGL->ourModel->CleanUpDestroyedObjects();
    }
}

// Call modules before each loop iteration
bool Application::PreUpdate()
{
    //Iterates the module list and calls PreUpdate on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->PreUpdate();
        if (!result) {
            break;
        }
    }

    return result;
}

// Call modules on each loop iteration
bool Application::DoUpdate()
{
    //Iterates the module list and calls Update on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->Update(dt);
        if (!result) {
            break;
        }
    }

    ProcessObjectSelection();
    return result;
}

// Call modules after each loop iteration
bool Application::PostUpdate()
{
    //Iterates the module list and calls PostUpdate on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->PostUpdate();
        if (!result) {
            break;
        }
    }

    return result;
}

void Application::ProcessObjectSelection() {

    // Comprobamos si se ha hecho click izquierdo
    if (input.get()->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN) {

        if (guiManager.get()->GetIO() && guiManager.get()->GetIO()->WantCaptureMouse) {
            return;
        }

        int mouseX, mouseY;
        // La clase Input solo tiene GetMousePosition() que devuelve SDL_FPoint. 
        // Si no tienes GetMousePosition(int&, int&), usa el miembro publico (o adaptalo)
        mouseX = input.get()->GetMousePosition().x;
        mouseY = input.get()->GetMousePosition().y;

        // Evitar seleccionar si ImGui está activo (esto es conceptual, requiere ImGui::IsWindowHovered())
        // Vamos a asumir que si el mouse está en (0,0) no seleccionamos.
        if (mouseX == 0 && mouseY == 0) return;

        // --- 1. Generación del Rayo ---
        glm::vec3 rayOrigin = camera.get()->cameraPos;
        int screenW = window.get()->width;
        int screenH = window.get()->height;

        glm::vec3 rayDir = camera.get()->ScreenPointToRay(
            (float)mouseX, (float)mouseY, screenW, screenH);

        float minDistance = std::numeric_limits<float>::max();
        std::shared_ptr<GameObject> hitObject = nullptr;

        // --- 2. Iteración y Test de Intersección ---
        for (auto& model : openGL.get()->modelObjects) {
            // Aseguramos que las matrices de transformación están actualizadas antes de usarlas
            for (auto& gameObject : model->gameObjects) {

                auto transformComp = gameObject->GetComponent(ComponentType::TRANSFORM);
                auto rendererComp = gameObject->GetComponent(ComponentType::MESH_RENDERER);

                if (!transformComp || !rendererComp) continue;

                auto transform = std::dynamic_pointer_cast<TransformComponent>(transformComp);
                auto renderer = std::dynamic_pointer_cast<RenderMeshComponent>(rendererComp);
                auto mesh = renderer->GetMesh();

                if (!transform || !mesh) continue;

                // Forzar actualización de la matriz (para evitar el error de matriz sucia)
                transform->Update();
                glm::mat4 modelMatrix = transform->GetGlobalTransform();

                // 2.1 Obtener AABB local
                glm::vec3 minLocal = mesh->meshAABB.min;
                glm::vec3 maxLocal = mesh->meshAABB.max;

                // 2.2 Transformación a World AABB (re-aligned)
                glm::vec3 worldAABB_min = glm::vec3(std::numeric_limits<float>::max());
                glm::vec3 worldAABB_max = glm::vec3(std::numeric_limits<float>::lowest());

                glm::vec3 localCorners[8] = {
                    glm::vec3(minLocal.x, minLocal.y, minLocal.z),
                    glm::vec3(maxLocal.x, minLocal.y, minLocal.z),
                    glm::vec3(minLocal.x, maxLocal.y, minLocal.z),
                    glm::vec3(maxLocal.x, maxLocal.y, minLocal.z),
                    glm::vec3(minLocal.x, minLocal.y, maxLocal.z),
                    glm::vec3(maxLocal.x, minLocal.y, maxLocal.z),
                    glm::vec3(minLocal.x, maxLocal.y, maxLocal.z),
                    glm::vec3(maxLocal.x, maxLocal.y, maxLocal.z)
                };

                for (int i = 0; i < 8; ++i) {
                    glm::vec4 worldPos = modelMatrix * glm::vec4(localCorners[i], 1.0f);
                    glm::vec3 worldCorner = glm::vec3(worldPos);
                    worldAABB_min = glm::min(worldAABB_min, worldCorner);
                    worldAABB_max = glm::max(worldAABB_max, worldCorner);
                }

                // --- 2.3 Test Ray-AABB (Slab Method) ---
                float tmin_ray = 0.0f;
                float tmax_ray = std::numeric_limits<float>::max();
                bool hit = true;

                for (int i = 0; i < 3; ++i) {
                    float invD = 1.0f / rayDir[i];
                    float t0 = (worldAABB_min[i] - rayOrigin[i]) * invD;
                    float t1 = (worldAABB_max[i] - rayOrigin[i]) * invD;

                    if (invD < 0.0f) {
                        std::swap(t0, t1);
                    }

                    tmin_ray = std::max(t0, tmin_ray);
                    tmax_ray = std::min(t1, tmax_ray);

                    if (tmax_ray <= tmin_ray) {
                        hit = false;
                        break;
                    }
                }

                if (hit) {
                    float hitDistance = tmin_ray;

                    // 2.4 Determinar el Objeto más Cercano
                    if (hitDistance < minDistance) {
                        minDistance = hitDistance;
                        hitObject = gameObject;
                    }
                }
            }
        }

        // --- 3. Gestión de la Selección ---
        guiManager.get()->SetSelectedObject(hitObject);
    }
}