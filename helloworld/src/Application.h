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
// Texture eliminado de aquí
class ResourceMesh;
class Model;
class GUIManager;
class Camera;

class Application
{
public:
	// Public method to get the instance of the Singleton
	static Application& GetInstance();

	void AddModule(std::shared_ptr<Module> module);

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool Update();

	// Called before quitting
	bool CleanUp();

	float GetDt() const { return dt; }
	int GetFPS() { return framesPerSecond; }

	void ProcessObjectSelection();

private:
	// Private constructor to prevent instantiation
	Application();

	// Delete copy constructor and assignment operator to prevent copying
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
	// std::shared_ptr<Texture> textures;  <-- ELIMINADO
	std::shared_ptr<Camera> camera;

	bool requestExit = false;

private:
	// Delta time
	float dt;
	// Frames since startup
	int frames;

	int frameCount = 0;
	int framesPerSecond = 0;
	int lastSecFrameCount = 0;

	float averageFps = 0.0f;
	int secondsSinceStartup = 0;

	// Maximun frame duration in miliseconds.
	int maxFrameDuration = 16;

	std::string gameTitle = "Vroom-Engine";
};