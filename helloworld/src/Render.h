#pragma once
#include "Module.h"
#include "SDL3/SDL.h"
#include "FileSystem.h"
#include <vector>

// Forward declaration
class Model;

class Render : public Module
{
public:

	Render();
	virtual ~Render();

	bool Awake();
	bool Start();
	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();
	bool CleanUp();

	void SetViewPort(const SDL_Rect& rect);
	void ResetViewPort();

	// Lista de modelos a dibujar
	std::vector<Model*> modelsToDraw;

	// Drawing 2D (SDL)
	bool DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section = NULL, float speed = 1.0f, double angle = 0, int pivotX = INT_MAX, int pivotY = INT_MAX) const;
	bool DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool filled = true, bool useCamera = true) const;
	bool DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool useCamera = true) const;
	bool DrawCircle(int x1, int y1, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool useCamera = true) const;

	void AddModel(Model* model);

	// DrawMesh ELIMINADO (Ya no usamos Mesh antigua, se usa ResourceMesh en RenderMeshComponent)

	void DrawGrid();
	void SetBackgroundColor(SDL_Color color);

public:
	SDL_Renderer* renderer;
	SDL_Rect camera;
	SDL_Rect viewport;
	SDL_Color background;

private:
	bool vsync = false;
};