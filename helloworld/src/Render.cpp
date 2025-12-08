#include "Application.h"
#include "Window.h"
#include "Render.h"
#include "Log.h"
#include "FileSystem.h"
#include "ResourceMesh.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Render::Render() : Module()
{
	name = "render";

	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
}

Render::~Render()
{
}

bool Render::Awake()
{
	LOG("Create SDL rendering context");
	bool ret = true;

	int scale = Application::GetInstance().window->GetScale();
	SDL_Window* window = Application::GetInstance().window->window;

	renderer = SDL_CreateRenderer(window, nullptr);

	if (renderer == NULL)
	{
		LOG("Could not create the renderer! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		if (vsync)
		{
			if (!SDL_SetRenderVSync(renderer, 1))
				LOG("Warning: could not enable vsync: %s", SDL_GetError());
		}

		camera.w = Application::GetInstance().window->width * scale;
		camera.h = Application::GetInstance().window->height * scale;
		camera.x = 0;
		camera.y = 0;
	}

	return ret;
}

bool Render::Start()
{
	if (!SDL_GetRenderViewport(renderer, &viewport))
	{
		LOG("SDL_GetRenderViewport failed: %s", SDL_GetError());
	}
	return true;
}

bool Render::PreUpdate()
{
	SDL_RenderClear(renderer);
	return true;
}

bool Render::Update(float dt)
{
	return true;
}

bool Render::PostUpdate()
{
	SDL_GL_SwapWindow(Application::GetInstance().window->window);
	return true;
}

bool Render::CleanUp()
{
	SDL_DestroyRenderer(renderer);
	return true;
}

void Render::SetBackgroundColor(SDL_Color color)
{
	background = color;
}

void Render::SetViewPort(const SDL_Rect& rect)
{
	SDL_SetRenderViewport(renderer, &rect);
}

void Render::ResetViewPort()
{
	SDL_SetRenderViewport(renderer, &viewport);
}

// 2D Drawing methods
bool Render::DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY) const
{
	bool ret = true;
	int scale = Application::GetInstance().window->GetScale();

	SDL_FRect rect;
	rect.x = (float)((int)(camera.x * speed) + x * scale);
	rect.y = (float)((int)(camera.y * speed) + y * scale);

	if (section != NULL)
	{
		rect.w = (float)(section->w * scale);
		rect.h = (float)(section->h * scale);
	}
	else
	{
		float tw = 0.0f, th = 0.0f;
		if (!SDL_GetTextureSize(texture, &tw, &th)) return false;
		rect.w = tw * scale;
		rect.h = th * scale;
	}

	const SDL_FRect* src = NULL;
	SDL_FRect srcRect;
	if (section != NULL)
	{
		srcRect.x = (float)section->x;
		srcRect.y = (float)section->y;
		srcRect.w = (float)section->w;
		srcRect.h = (float)section->h;
		src = &srcRect;
	}

	SDL_FPoint* p = NULL;
	SDL_FPoint pivot;
	if (pivotX != INT_MAX && pivotY != INT_MAX)
	{
		pivot.x = (float)pivotX;
		pivot.y = (float)pivotY;
		p = &pivot;
	}

	if (SDL_RenderTextureRotated(renderer, texture, src, &rect, angle, p, SDL_FLIP_NONE) != 0)
	{
		ret = false;
	}

	return ret;
}

bool Render::DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
	bool ret = true;
	int scale = Application::GetInstance().window->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_FRect rec;
	if (use_camera)
	{
		rec.x = (float)((int)(camera.x + rect.x * scale));
		rec.y = (float)((int)(camera.y + rect.y * scale));
		rec.w = (float)(rect.w * scale);
		rec.h = (float)(rect.h * scale);
	}
	else
	{
		rec.x = (float)(rect.x * scale);
		rec.y = (float)(rect.y * scale);
		rec.w = (float)(rect.w * scale);
		rec.h = (float)(rect.h * scale);
	}

	if ((filled ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderRect(renderer, &rec)) != 0)
	{
		ret = false;
	}

	return ret;
}

bool Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	int scale = Application::GetInstance().window->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	float X1, Y1, X2, Y2;

	if (use_camera)
	{
		X1 = (float)(camera.x + x1 * scale);
		Y1 = (float)(camera.y + y1 * scale);
		X2 = (float)(camera.x + x2 * scale);
		Y2 = (float)(camera.y + y2 * scale);
	}
	else
	{
		X1 = (float)(x1 * scale);
		Y1 = (float)(y1 * scale);
		X2 = (float)(x2 * scale);
		Y2 = (float)(y2 * scale);
	}

	if (SDL_RenderLine(renderer, X1, Y1, X2, Y2) != 0)
	{
		ret = false;
	}

	return ret;
}

bool Render::DrawCircle(int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	int scale = Application::GetInstance().window->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_FPoint points[360];
	float factor = (float)M_PI / 180.0f;
	float cx = (float)((use_camera ? camera.x : 0) + x * scale);
	float cy = (float)((use_camera ? camera.y : 0) + y * scale);

	for (int i = 0; i < 360; ++i)
	{
		points[i].x = cx + (float)(radius * cos(i * factor));
		points[i].y = cy + (float)(radius * sin(i * factor));
	}

	if (SDL_RenderPoints(renderer, points, 360) != 0)
	{
		ret = false;
	}

	return ret;
}

void Render::AddModel(Model* model) {
	modelsToDraw.push_back(model);
}

void Render::DrawGrid() {
	float lineX = -100.0f;
	float lineZ = -100.0f;
	float lineLength = 100.0f;
	for (int i = 0; i < 1000; i++) {
		glLineWidth(1.0f);
		glBegin(GL_LINES);
		glVertex3f(-lineLength, 0.0f, lineZ);
		glVertex3f(lineLength, 0.0f, lineZ);
		glVertex3f(lineX, 0.f, -lineLength);
		glVertex3f(lineX, 0.f, lineLength);
		glEnd();
		lineX++;
		lineZ++;
	}
}