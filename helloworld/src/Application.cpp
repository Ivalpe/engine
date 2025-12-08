#include "Application.h"
#include <iostream>
#include "Log.h"

#include "Window.h"
#include "Input.h"
#include "Render.h"
#include "OpenGL.h"
#include "FileSystem.h"
// #include "Textures.h" <-- ELIMINADO

#include "GUIManager.h"
#include "RenderMeshComponent.h"
#include "TransformComponent.h"
#include "Camera.h"
#include "ResourceMesh.h"
#include "ResMan.h" // Añadimos ResourceManager si no estaba, ya que gestiona recursos ahora

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
    // textures = std::make_shared<Texture>(); <-- ELIMINADO
    camera = std::make_shared<Camera>();

    // Init ResourceManager Singleton (Opcional, si tiene Init explícito)
    // ResourceManager::GetInstance().Init(); 

    // Ordered for awake / Start / Update
    // Reverse order of CleanUp
    AddModule(std::static_pointer_cast<Module>(window));
    AddModule(std::static_pointer_cast<Module>(guiManager));
    AddModule(std::static_pointer_cast<Module>(input));
    // AddModule(std::static_pointer_cast<Module>(textures)); <-- ELIMINADO
    AddModule(std::static_pointer_cast<Module>(camera));

    // Render last 
    AddModule(std::static_pointer_cast<Module>(openGL));
    AddModule(std::static_pointer_cast<Module>(fileSystem));
    AddModule(std::static_pointer_cast<Module>(render));
}