#include "Application.h"
#include "Input.h"
#include "Window.h"
#include "GUIManager.h"
#include "Log.h"
#include "OpenGL.h"

// Includes de sistema y contenedores PRIMERO para evitar errores de plantillas
#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <algorithm>

// Componentes y lógica del motor
#include "GameObject.h"
#include "Component.h"
#include "RenderMeshComponent.h"
#include "Model.h"
#include "Render.h"
#include "ModelImporter.h" // Necesario para importar al soltar archivos
#include "ResMan.h"

#include "SDL3/SDL.h"

using namespace std;

Input::Input() : Module()
{
	name = "input";

	keyboard = new KeyState[MAX_KEYS];
	memset(keyboard, KEY_IDLE, sizeof(KeyState) * MAX_KEYS);
	memset(mouseButtons, KEY_IDLE, sizeof(KeyState) * NUM_MOUSE_BUTTONS);
}

Input::~Input()
{
	delete[] keyboard;
}

bool Input::Awake()
{
	LOG("Init SDL input event system");
	bool ret = true;
	SDL_Init(0);

	if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Input::Start()
{
	SDL_StopTextInput();
	return true;
}

bool Input::PreUpdate()
{
	SDL_Event event;

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	for (int i = 0; i < MAX_KEYS; ++i)
	{
		if (keys[i] == 1)
		{
			if (keyboard[i] == KEY_IDLE)
				keyboard[i] = KEY_DOWN;
			else
				keyboard[i] = KEY_REPEAT;
		}
		else
		{
			if (keyboard[i] == KEY_REPEAT || keyboard[i] == KEY_DOWN)
				keyboard[i] = KEY_UP;
			else
				keyboard[i] = KEY_IDLE;
		}
	}

	for (int i = 0; i < NUM_MOUSE_BUTTONS; ++i)
	{
		if (mouseButtons[i] == KEY_DOWN)
			mouseButtons[i] = KEY_REPEAT;

		if (mouseButtons[i] == KEY_UP)
			mouseButtons[i] = KEY_IDLE;
	}

	while (SDL_PollEvent(&event) != 0)
	{
		// Pasar eventos a ImGui
		Application::GetInstance().guiManager->ProcessEvents(event);

		switch (event.type)
		{
		case SDL_EVENT_QUIT:
			windowEvents[WE_QUIT] = true;
			break;

		case SDL_EVENT_WINDOW_RESIZED:
		case SDL_EVENT_WINDOW_SIZE_CHANGED:
			Application::GetInstance().window->SetWindowSize(glm::vec2(event.window.data1, event.window.data2));
			Application::GetInstance().render->ResetViewPort();
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			mouseButtons[event.button.button - 1] = KEY_DOWN;
			break;

		case SDL_EVENT_MOUSE_BUTTON_UP:
			mouseButtons[event.button.button - 1] = KEY_UP;
			break;

		case SDL_EVENT_MOUSE_MOTION:
			mouseMotionX = event.motion.xrel;
			mouseMotionY = event.motion.yrel;
			mouseX = event.motion.x;
			mouseY = event.motion.y;
			break;

			// --- DRAG AND DROP IMPLEMENTATION ---
		case SDL_EVENT_DROP_FILE:
		{
			char* droppedFile = event.drop.file;
			if (droppedFile != nullptr)
			{
				std::string path(droppedFile);
				LOG("File Dropped: %s", path.c_str());

				// Obtener extensión para saber qué hacer
				std::string extension = std::filesystem::path(path).extension().string();
				// Convertir a minúsculas para comparar
				std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

				if (extension == ".fbx" || extension == ".obj")
				{
					// 1. Crear nuevo Modelo (esto dispara el ModelImporter internamente en el constructor de Model)
					Model* newModel = new Model(path);

					// 2. Añadir a la lista de OpenGL para renderizado y actualización
					if (Application::GetInstance().openGL) {
						Application::GetInstance().openGL->modelObjects.push_back(newModel);

						// 3. Añadir root object a la jerarquía del GUI
						if (newModel->GetRootGameObject()) {
							Application::GetInstance().guiManager->sceneObjects.push_back(newModel->GetRootGameObject());
						}
					}
				}
				else if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".tga")
				{
					// TODO: Implementar Drag & Drop de texturas
					// Ahora deberías usar ResourceManager::CreateResource(ResourceType::TEXTURE, ...)
					// O TextureImporter::Import(...)
					LOG("Texture drop detected. Logic pending implementation of TextureImporter.");
				}
				else
				{
					LOG("Unknown file format dropped.");
				}

				// Liberar memoria asignada por SDL
				// SDL_free(droppedFile); // SDL3 puede requerir esto o no dependiendo de la versión exacta, revisar docs.
			}
			break;
		}
		}
	}

	return true;
}

bool Input::CleanUp()
{
	LOG("Quitting SDL event subsystem");
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
	return true;
}