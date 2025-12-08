#include "Application.h"
#include "Window.h"
#include "GUIElement.h"
#include "Log.h"
#include "FileSystem.h"
#include "GUIManager.h"
#include "SystemInfo.h"
#include "OpenGL.h"
#include "Model.h"

#include "TransformComponent.h"
#include "RenderMeshComponent.h"
#include "MaterialComponent.h"
#include "ResourceTexture.h" // Usamos el nuevo recurso
#include "ResourceMesh.h"    // Usamos el nuevo recurso
#include "Render.h"
#include "ResMan.h"          // Necesario si queremos usar recursos

#include <SDL3/SDL_opengl.h>
#include <glm/glm.hpp>
#include <assimp/version.h>
#include <fmt/core.h>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>
#include <filesystem> // Necesario para el panel de Assets

GUIElement::GUIElement(ElementType t, GUIManager* m)
{
	type = t;
	manager = m;
	// Inicializamos la ruta de assets
	currentPath = "Assets";
}

GUIElement:: ~GUIElement()
{

}

void GUIElement::ElementSetUp()
{
	switch (type) {
	case ElementType::Additional:
		//additional handles popup windows
		AboutSetUp();
		break;
	case ElementType::MenuBar:
		MenuBarSetUp();
		break;
	case ElementType::Console:
		if (Application::GetInstance().guiManager.get()->showConsole) ConsoleSetUp(&Application::GetInstance().guiManager.get()->showConsole);
		break;
	case ElementType::Config:
		if (Application::GetInstance().guiManager.get()->showConfig) ConfigSetUp(&Application::GetInstance().guiManager.get()->showConfig);
		break;
	case ElementType::Hierarchy:
		if (Application::GetInstance().guiManager.get()->showHierarchy) HierarchySetUp(&Application::GetInstance().guiManager.get()->showHierarchy);
		break;
	case ElementType::Inspector:
		if (Application::GetInstance().guiManager.get()->showInspector) InspectorSetUp(&Application::GetInstance().guiManager.get()->showInspector);
		break;
	case ElementType::Assets: // NUEVO CASO
		if (Application::GetInstance().guiManager.get()->showAssets) AssetsSetUp(&Application::GetInstance().guiManager.get()->showAssets);
		break;
	default:
		LOG("No GUIType detected.");
		break;
	}
}

//type set ups
void GUIElement::MenuBarSetUp()
{
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Exit")) {
				//handle exit
				Application::GetInstance().requestExit = true;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {
			//handle view
			if (ImGui::MenuItem("Console", nullptr, Application::GetInstance().guiManager.get()->showConsole)) {
				bool set = !Application::GetInstance().guiManager.get()->showConsole;
				Application::GetInstance().guiManager.get()->showConsole = set;
			}
			if (ImGui::MenuItem("Configuration", nullptr, Application::GetInstance().guiManager.get()->showConfig)) {
				bool set = !Application::GetInstance().guiManager.get()->showConfig;
				Application::GetInstance().guiManager.get()->showConfig = set;
			}
			if (ImGui::MenuItem("Hierarchy", nullptr, Application::GetInstance().guiManager.get()->showHierarchy)) {
				bool set = !Application::GetInstance().guiManager.get()->showHierarchy;
				Application::GetInstance().guiManager.get()->showHierarchy = set;
			}
			if (ImGui::MenuItem("Inspector", nullptr, Application::GetInstance().guiManager.get()->showInspector)) {
				bool set = !Application::GetInstance().guiManager.get()->showInspector;
				Application::GetInstance().guiManager.get()->showInspector = set;
			}
			// Toggle para Assets
			if (ImGui::MenuItem("Assets", nullptr, Application::GetInstance().guiManager.get()->showAssets)) {
				bool set = !Application::GetInstance().guiManager.get()->showAssets;
				Application::GetInstance().guiManager.get()->showAssets = set;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("Documentation")) {
				//handle documentation
				SDL_OpenURL("https://github.com/KaiCaire/vroom-engine?tab=readme-ov-file#readme");

			}
			if (ImGui::MenuItem("Report a Bug")) {
				//handle report 
				SDL_OpenURL("https://github.com/KaiCaire/vroom-engine/issues");
			}
			if (ImGui::MenuItem("Latest Release")) {
				//handle release
				SDL_OpenURL("https://github.com/KaiCaire/vroom-engine/releases");
			}
			if (ImGui::MenuItem("About")) {
				//handle about window
				Application::GetInstance().guiManager.get()->showAboutPopup = true;
			}
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}

void GUIElement::AboutSetUp() {
	if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

		//Text
		ImGui::Text("VroomEngine v.1");
		ImGui::Separator();
		ImGui::Text("Developed by:");
		ImGui::BulletText("Ivan Alvarez");
		ImGui::BulletText("Kai Caire");
		ImGui::BulletText("Lara Guevara");
		ImGui::BulletText("Bernat Loza");
		ImGui::Separator();
		ImGui::Text("Developed using:");
		ImGui::BulletText("vcpkg");
		ImGui::BulletText("assimp");
		ImGui::BulletText("stb");
		ImGui::BulletText("glm");
		ImGui::BulletText("imgui");
		ImGui::BulletText("glad (for OpenGL)");
		ImGui::BulletText("sdl-3 & sdl-3.image");
		ImGui::NewLine();
		ImGui::Text("MIT License Copyright(c) 2025");

		ImGui::Spacing();
		ImGui::Separator();

		//Close button
		if (ImGui::Button("Close Window", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void GUIElement::ConsoleSetUp(bool* show) {
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

	//initialize for scroll button
	bool scrollToBottom = false;

	//initial states
	ImGui::SetNextWindowDockID(0, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);

	//check if we should show it
	if (!ImGui::Begin("Console", show, window_flags))
	{
		//if not -> end here
		ImGui::End();
		return;
	}

	//Button controls
	//Clear logs
	if (ImGui::Button("Clear")) ClearLogs();
	ImGui::SameLine();

	//Scroll to the bottom
	if (ImGui::Button("Go to Bottom")) scrollToBottom = true;
	ImGui::Separator();

	//log messages area
	//create area for the console
	ImGui::BeginChild("ScrollArea", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	auto logs = GetLogBuffer();
	//show log messages
	for (const auto& line : logs) ImGui::TextUnformatted(line.c_str());

	if (scrollToBottom) {
		ImGui::SetScrollHereY(1.0f);
		scrollToBottom = false;
	}

	ImGui::EndChild();
	ImGui::End();
}

void GUIElement::ConfigSetUp(bool* show) {
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

	//initial states
	ImGui::SetNextWindowDockID(0, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);

	//check if we should show it
	if (!ImGui::Begin("Configuration", show, window_flags))
	{
		//if not -> end here
		ImGui::End();
		return;
	}

	//show fps
	//ImGui::Text("FPS: %d", Application::GetInstance().GetFPS());
	ImGui::Text("FPS: %.1f (%.1f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Separator();

	//variable config
	ImGui::Text("Variables:");
	//window full screen
	//check if window is fullscreen
	bool fullscreen = Application::GetInstance().window.get()->isFullscreen;
	if (ImGui::Checkbox("Fullscreen", &fullscreen)) {
		Application::GetInstance().window.get()->SetFullScreen(fullscreen);
	}
	//window resolution
	if (!fullscreen) {
		//get resolutions and current resolution from window
		std::vector<glm::vec2> options = Application::GetInstance().window.get()->resolutions;
		glm::vec2 current = Application::GetInstance().window.get()->currentRes;

		//find index of current resolution
		int index = 0;

		//setup dropdown menu
		ImGui::Text("Resolution");
		ImGui::SameLine();
		if (ImGui::BeginCombo("##Resolution", (std::to_string((int)current.x) + "x" + std::to_string((int)current.y)).c_str())) {
			for (int i = 0; i < options.size(); i++) {
				//find selected resoltion
				bool selected = (current == options[i]);

				//create option label
				std::string label = (std::to_string((int)options[i].x) + "x" + std::to_string((int)options[i].y));
				if (ImGui::Selectable(label.c_str(), selected)) {
					//apply resolution
					current = options[i];
					Application::GetInstance().window.get()->SetWindowSize(current);
				}

				if (selected) ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}
	}
	ImGui::Separator();

	//hardware and memory consuption
	ImGui::Text("Hardware and Memory Information:");
	ImGui::BulletText("Memory Consumption: %.2f MB", GetMemoryUsageMB());
	ImGui::BulletText("CPU Cores: %u", GetCPUCoreCount());
	ImGui::Separator();

	//software versions 
	ImGui::Text("Software Versions:");
	ImGui::BulletText("SDL3: %d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION);

	// OpenGL
	const char* glVer = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	ImGui::BulletText("OpenGL: %s", glVer ? glVer : "Unknown");

	ImGui::BulletText("ImGui: %s", IMGUI_VERSION);
	ImGui::BulletText("GLM: %d.%d.%d", GLM_VERSION_MAJOR, GLM_VERSION_MINOR, GLM_VERSION_PATCH);
	ImGui::BulletText("Assimp: %d.%d.%d", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionRevision());
	ImGui::BulletText("fmt: %d.%d.%d", FMT_VERSION / 10000, (FMT_VERSION / 100) % 100, FMT_VERSION % 100);

	ImGui::End();
}

void GUIElement::HierarchySetUp(bool* show)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

	//initial states
	ImGui::SetNextWindowDockID(0, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);

	//check if we should show it
	if (!ImGui::Begin("Hierarchy", show, window_flags))
	{
		//if not -> end here
		ImGui::End();
		return;
	}

	//create objects (minim cube)
	if (ImGui::BeginMenu("Create...")) {
		if (ImGui::MenuItem("Empty")) {
			//Create empty 
			auto empty = new Model();
			// Application::GetInstance().render->AddModel(empty); // Render ya no gestiona Models directamente así en algunos casos, revisar

			//add empty model to lists
			Application::GetInstance().openGL.get()->modelObjects.push_back(empty);
			Application::GetInstance().guiManager.get()->sceneObjects.push_back(empty->GetRootGameObject());

		}
		if (ImGui::MenuItem("Cube")) {
			Model* defaultCube = Application::GetInstance().openGL->CreateCube();
			Application::GetInstance().render->AddModel(defaultCube);
		}
		ImGui::EndMenu();
	}

	ImGui::Separator();

	//game objects
	//get root level objects
	for (auto& obj : manager->sceneObjects)
	{
		//check for game objects with no parent
		if (obj && obj->IsActive() && !obj->GetParent()) DrawNode(obj, manager->selectedObject);
	}

	ImGui::End();
}

void GUIElement::DrawNode(const std::shared_ptr<GameObject>& obj, std::shared_ptr<GameObject>& selected) {
	//make sure obj is not set for deletion
	if (!obj) return;
	if (obj.get()->IsMarkedForDestroy()) return;

	//setup tree structure (add arrows to expandable objects, make it so they show as selected)
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
		(obj == selected ? ImGuiTreeNodeFlags_Selected : 0) |
		(obj->GetChildren().empty() ? ImGuiTreeNodeFlags_Leaf : 0);

	//create node and check if opened
	bool opened = ImGui::TreeNodeEx((void*)obj.get(), flags, "%s", obj->GetName().c_str());

	//check if object has been selected
	if (ImGui::IsItemClicked()) {
		if (selected != nullptr) selected->isSelected = false;
		selected = obj;
	}

	if (selected != nullptr) {
		selected->isSelected = true;
	}

	//right click to delete object
	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Delete")) {
			manager->AddToDeleteQueue(obj);
			if (selected == obj) selected = nullptr;
		}
		ImGui::EndPopup();
	}

	//show children 
	if (opened)
	{
		for (auto& child : obj->GetChildren()) DrawNode(child, selected);
		ImGui::TreePop();
	}
}

// NUEVA FUNCIÓN: Panel de Assets
void GUIElement::AssetsSetUp(bool* show)
{
	if (!ImGui::Begin("Assets", show))
	{
		ImGui::End();
		return;
	}

	// Navegación hacia atrás
	if (currentPath != std::filesystem::path("Assets"))
	{
		if (ImGui::Button(".."))
		{
			currentPath = currentPath.parent_path();
		}
		ImGui::Separator();
	}

	// Listar archivos
	if (std::filesystem::exists(currentPath)) {
		for (const auto& entry : std::filesystem::directory_iterator(currentPath))
		{
			const auto& path = entry.path();
			std::string filename = path.filename().string();

			if (entry.is_directory())
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Amarillo para carpetas
				if (ImGui::Selectable(filename.c_str()))
				{
					currentPath = path;
				}
				ImGui::PopStyleColor();
			}
			else
			{
				ImGui::Selectable(filename.c_str());
				// TODO: Implement Drag & Drop Source here
			}
		}
	}

	ImGui::End();
}

void GUIElement::InspectorSetUp(bool* show)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

	//initial states
	ImGui::SetNextWindowDockID(0, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);

	//check if we should show it
	if (!ImGui::Begin("Inspector", show, window_flags))
	{
		//if not -> end here
		ImGui::End();
		return;
	}

	//check if a game object is selected
	auto selected = manager->selectedObject;

	if (selected) {
		//show game object name
		char buffer[128];
		strncpy(buffer, selected->GetName().c_str(), sizeof(buffer));
		buffer[sizeof(buffer) - 1] = '\0'; // Asegurar null termination

		if (ImGui::InputText("Name", buffer, sizeof(buffer))) selected->SetName(buffer);

		//transform
		//get transform component
		auto transform = std::dynamic_pointer_cast<TransformComponent>(selected->GetComponent(ComponentType::TRANSFORM));
		if (transform) {
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
				glm::vec3 pos = transform->GetWorldPosition();
				glm::vec3 rot = glm::degrees(glm::eulerAngles(transform->GetWorldRotation()));
				glm::vec3 scale = transform->GetWorldScale();

				// Nota: Editar directamente la transformación global es complejo por la jerarquía.
				// Aquí solo mostramos valores informativos.
				ImGui::Text("Global Position: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);

				// Para editar, deberías exponer el LocalTransform en TransformComponent.
				// ImGui::DragFloat3("Position", &localPos.x, 0.1f); etc.
			}
		}

		// --- MESH ---
		auto meshComponent = std::dynamic_pointer_cast<RenderMeshComponent>(selected->GetComponent(ComponentType::MESH_RENDERER));

		if (meshComponent) {
			// Usamos ResourceMesh ahora
			std::shared_ptr<ResourceMesh> mesh = meshComponent->GetMesh();

			if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
				if (mesh && mesh->IsLoaded()) {
					// Mostramos información disponible en ResourceMesh
					ImGui::Text("Resource UID: %u", mesh->GetUID());
					ImGui::Text("Vertices: %d", mesh->vertexCount);
					ImGui::Text("Indices: %d", mesh->indexCount);
					ImGui::Text("VAO ID: %d", mesh->VAO);

					ImGui::Separator();
					ImGui::TextDisabled("Normals visualization unavailable (Optimized VRAM)");
				}
				else {
					ImGui::TextColored(ImVec4(1, 0, 0, 1), "No Mesh Loaded");
				}
			}

			// --- TEXTURE ---
			auto materialComp = std::dynamic_pointer_cast<MaterialComponent>(selected->GetComponent(ComponentType::MATERIAL));

			if (materialComp && ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {

				auto currentTex = materialComp->GetDiffuseMap();

				if (currentTex && currentTex->IsLoaded()) {
					ImGui::Text("Diffuse Texture:");
					ImGui::Image((ImTextureID)(intptr_t)currentTex->textureID, ImVec2(100, 100));

					ImGui::Text("Texture ID: %u", currentTex->textureID);
					ImGui::BulletText("Size: %dx%d", currentTex->width, currentTex->height);
					ImGui::BulletText("Path: %s", currentTex->assetsPath.c_str());
				}
				else {
					ImGui::Text("No Diffuse Texture assigned.");
				}

				// Checker Texture Logic (Desactivada temporalmente por refactor de módulo Textures)
				/*
				auto parentModel = manager->FindGameObjectModel(selected);
				if (parentModel) {
					if (ImGui::Checkbox("Use Default Texture", &parentModel->useDefaultTexture)) {
						// Logic needs to be updated to use ResourceManager::Load("checkers.png")
						// instead of Application::GetInstance().textures
					}
				}
				*/
			}
		}
	}
	else {
		ImGui::Text("No GameObject selected.");
	}

	ImGui::End();
}