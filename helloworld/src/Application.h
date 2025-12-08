#pragma once

#include <memory>
#include <list>
#include "Module.h"

// Modules
class Window;
class Input;
class Render;
class OpenGL;
class FileSystem;
// class Texture;  <-- ELIMINAR ESTA LÍNEA (Ya no es un módulo)
class Mesh;
class Model;
class GUIManager;
class Camera;

class Application
{
public:
	static Application& GetInstance();
	void AddModule(std::shared_ptr<Module> module);

	bool Awake();
	bool Start();
	bool Update();
	bool CleanUp();

	float GetDt() const { return dt; }
	int GetFPS() { return framesPerSecond; }

	void ProcessObjectSelection();

private:
	Application();
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	void PrepareUpdate();
	void FinishUpdate();
	bool PreUpdate();
	bool DoUpdate();
	bool PostUpdate();

	std::list<std::shared_ptr<Module>> moduleList;

public:
	enum EngineState
	{
		CREATE = 1,
		AWAKE,
		START,
		LOOP,
		CLEAN,
		FAIL,
		EXIT
	};

	// Modules
	std::shared_ptr<Window> window;
	std::shared_ptr<GUIManager> guiManager;
	std::shared_ptr<Input> input;
	std::shared_ptr<Render> render;
	std::shared_ptr<OpenGL> openGL;
	std::shared_ptr<FileSystem> fileSystem;
	// std::shared_ptr<Texture> textures; <-- ELIMINAR ESTA LÍNEA
	std::shared_ptr<Camera> camera;

	bool requestExit = false;

private:
	float dt;
	int framesPerSecond;
	int frameCount;
	float lastFrameTime;
};