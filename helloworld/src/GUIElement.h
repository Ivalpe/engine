#pragma once

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include "SDL3/SDL.h"

#include "Module.h"
#include "FileSystem.h"
#include <memory>
#include <filesystem> // Necesario para std::filesystem::path

class GameObject; // Forward declaration
class GUIManager;

// Añadido 'Assets' al final del Enum
enum ElementType { Additional, MenuBar, Console, Config, Hierarchy, Inspector, Assets };

class GUIElement {
public:
	GUIElement(ElementType t, GUIManager* m);
	~GUIElement();

	void ElementSetUp();

	//type set ups
	void MenuBarSetUp();
	void AboutSetUp();
	void ConsoleSetUp(bool* show);
	void ConfigSetUp(bool* show);
	void HierarchySetUp(bool* show);
	void InspectorSetUp(bool* show);

	// Nueva función para el panel de Assets
	void AssetsSetUp(bool* show);

	//other
	void DrawNode(const std::shared_ptr<GameObject>& obj, std::shared_ptr<GameObject>& selected);


private:
	ElementType type;
	GUIManager* manager;

	// Variable para guardar la ruta actual del explorador de archivos
	std::filesystem::path currentPath;
};