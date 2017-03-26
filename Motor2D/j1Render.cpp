#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Window.h"
#include "j1Render.h"
#include "j1Map.h"

#define VSYNC true

j1Render::j1Render() : j1Module()
{
	name.assign("renderer");
	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
}

// Destructor
j1Render::~j1Render()
{}

// Called before render is available
bool j1Render::Awake(pugi::xml_node& config)
{
	LOG("Create SDL rendering context");
	bool ret = true;
	// load flags
	Uint32 flags = SDL_RENDERER_ACCELERATED;

	if(config.child("vsync").attribute("value").as_bool(true) == true)
	{
		flags |= SDL_RENDERER_PRESENTVSYNC;
		LOG("Using vsync");
	}

	renderer = SDL_CreateRenderer(App->win->window, -1, flags);

	if(renderer == NULL)
	{
		LOG("Could not create the renderer! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		camera = new Camera(SDL_Rect{ 0, 0, App->win->screen_surface->w, App->win->screen_surface->h });
	}

	return ret;
}

// Called before the first frame
bool j1Render::Start()
{
	LOG("render start");
	// back background
	SDL_RenderGetViewport(renderer, &viewport);
	return true;
}

// Called each loop iteration
bool j1Render::PreUpdate()
{
	SDL_RenderClear(renderer);
	return true;
}

bool j1Render::PostUpdate()
{
	SDL_SetRenderDrawColor(renderer, background.r, background.g, background.g, background.a);
	SDL_RenderPresent(renderer);
	return true;
}

// Called before quitting
bool j1Render::CleanUp()
{
	LOG("Destroying SDL render");
	SDL_DestroyRenderer(renderer);
	return true;
}

// Load Game State
bool j1Render::Load(pugi::xml_node& data)
{
	camera->SetPosition(iPoint(data.child("camera").attribute("x").as_int(), data.child("camera").attribute("y").as_int()));
	return true;
}

// Save Game State
bool j1Render::Save(pugi::xml_node& data) const
{
	pugi::xml_node cam = data.append_child("camera");

	cam.append_attribute("x") =  App->render->camera->GetPosition().x;
	cam.append_attribute("y") =  App->render->camera->GetPosition().y;

	return true;
}

void j1Render::SetBackgroundColor(SDL_Color color)
{
	background = color;
}

void j1Render::GetDimensionsFromTexture(SDL_Texture* tex, int& w, int& h)
{
	SDL_QueryTexture(tex, NULL, NULL, &w, &h);
}

iPoint j1Render::WorldToScreen(int x, int y) const
{
	iPoint ret;
	int scale = App->win->GetScale();

	ret.x = (x + camera->GetPosition().x / scale);
	ret.y = (y + camera->GetPosition().y / scale);

	return ret;
}

void j1Render::SetViewPort(const SDL_Rect& rect)
{
	SDL_RenderSetViewport(renderer, &rect);
}

void j1Render::ResetViewPort()
{
	SDL_RenderSetViewport(renderer, &viewport);
}

iPoint j1Render::ScreenToWorld(int x, int y) const
{
	iPoint ret;
	int scale = App->win->GetScale();

	ret.x = (x -  App->render->camera->GetPosition().x / scale);
	ret.y = (y -  App->render->camera->GetPosition().y / scale);

	return ret;
}

// Blit to screen
bool j1Render::Blit(SDL_Texture* texture, int x, int y, const SDL_Rect* section, SDL_RendererFlip flip, int pivot_x, int pivot_y, float speed, double angle) const
{
	bool ret = true;
	uint scale = App->win->GetScale();

	SDL_Rect rect;
	rect.x = (int)( App->render->camera->GetPosition().x * speed) + x * scale;
	rect.y = (int)( App->render->camera->GetPosition().y * speed) + y * scale;

	iPoint screen_position = App->render->WorldToScreen(x, y);

	if (App->render->camera->InsideRenderTarget(screen_position.x, screen_position.y))
	{
		if (section != NULL)
		{
			rect.w = section->w;
			rect.h = section->h;
		}
		else
		{
			SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
		}

		if (flip == SDL_FLIP_HORIZONTAL)
		{
			rect.x -= (rect.w - pivot_x);
			rect.y -= pivot_y;
		}

		else if (flip == SDL_FLIP_VERTICAL)
		{
			rect.x -= pivot_x;
			rect.y -= (rect.h - pivot_y);
		}

		else if (flip == SDL_FLIP_NONE)
		{
			rect.x -= pivot_x;
			rect.y -= pivot_y;
		}

		rect.w *= scale;
		rect.h *= scale;

		SDL_Point* p = NULL;
		SDL_Point pivot;

		if (pivot_x != INT_MAX && pivot_y != INT_MAX)
		{
			pivot.x = pivot_x;
			pivot.y = pivot_y;
			p = &pivot;
		}
		rect = App->render->camera->GetZoomedRect(SDL_Rect{ screen_position.x,screen_position.y,rect.w,rect.h });
		if (SDL_RenderCopyEx(renderer, texture, section, &rect, angle, p, (SDL_RendererFlip)flip) != 0)
		{
			LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
			ret = false;
		}
	}

	return ret;
}

bool j1Render::DrawQuad(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
	bool ret = true;
	uint scale = App->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_Rect rec(rect);
	if(use_camera)
	{
		rec.x = (int)( App->render->camera->GetPosition().x + rect.x * scale);
		rec.y = (int)( App->render->camera->GetPosition().y + rect.y * scale);
		rec.w *= scale;
		rec.h *= scale;
	}

	int result = (filled) ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderDrawRect(renderer, &rec);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool j1Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	uint scale = App->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;

	if(use_camera)
		result = SDL_RenderDrawLine(renderer, camera->GetPosition().x + x1 * scale,  App->render->camera->GetPosition().y + y1 * scale,  App->render->camera->GetPosition().x + x2 * scale,  App->render->camera->GetPosition().y + y2 * scale);
	else
		result = SDL_RenderDrawLine(renderer, x1 * scale, y1 * scale, x2 * scale, y2 * scale);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool j1Render::DrawCircle(int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	uint scale = App->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;
	SDL_Point points[360];

	float factor = (float)M_PI / 180.0f;

	for(uint i = 0; i < 360; ++i)
	{
		points[i].x = (int)(x + radius * cos(i * factor));
		points[i].y = (int)(y + radius * sin(i * factor));
	}

	result = SDL_RenderDrawPoints(renderer, points, 360);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

SDL_Rect j1Render::ZoomedRect(const SDL_Rect &starting_rect) const
{
	SDL_Rect rect = starting_rect;

	rect.w = rect.w * (1.0f + zoom / 100.0f);

	rect.h = rect.h * (1.0f + zoom / 100.0f);

	rect.x = (rect.x - viewport.w / 2.0f) * (1.0f + (zoom / 100.0f)) + viewport.w / 2.0f;

	rect.y = (rect.y - viewport.h / 2.0f) * (1.0f + (zoom / 100.0f)) + viewport.h / 2.0f;

	return rect;
}

Camera::Camera(Camera & copy) : moving(false), destination(iPoint(0, 0)), speed(0), x_movement(0), y_movement(0), zoom(0.0f), opacity(255), frames_to_black(0), frames_in_black(0), frames_to_light(0), opacity_delta(0), follow(nullptr)
{
	view_port.x = copy.view_port.x;
	view_port.y = copy.view_port.y;
	view_port.w = copy.view_port.w;
	view_port.h = copy.view_port.h;
}

Camera::Camera(SDL_Rect & rect) : moving(false), destination(iPoint(0, 0)), speed(0), x_movement(0), y_movement(0), zoom(0.0f), opacity(255), frames_to_black(0), frames_in_black(0), frames_to_light(0), opacity_delta(0), follow(nullptr)
{
	view_port.x = rect.x;
	view_port.y = rect.y;
	view_port.w = rect.w;
	view_port.h = rect.h;
}

void Camera::SetPosition(iPoint pos)
{
	view_port.x = pos.x;
	view_port.y = pos.y;
}

void Camera::SetSize(int w, int h)
{
	view_port.w = w;
	view_port.h = h;
}

const iPoint Camera::GetPosition() const
{
	return iPoint(view_port.x, view_port.y);
}

const int Camera::GetWidth() const
{
	return view_port.w;
}

const int Camera::GetHeight() const
{
	return view_port.h;
}

const SDL_Rect Camera::GetVP() const
{
	return view_port;
}

const iPoint Camera::GetCenter() const
{
	return iPoint((view_port.x + view_port.w / 2), (view_port.y + view_port.h / 2));
}

const float Camera::GetOpacity() const
{
	return opacity;
}

bool Camera::InsideRenderTarget(int x, int y)
{
	x = (x - view_port.w / 2.0f) * (1.0f + (zoom / 100.0f)) + view_port.w / 2.0f;

	y = (y - view_port.h / 2.0f) * (1.0f + (zoom / 100.0f)) + view_port.h / 2.0f;

	int zoomed_tile_width = App->map->data.tile_width *(1.0f + zoom / 100.0f);
	int zoomed_tile_height = App->map->data.tile_height *(1.0f + zoom / 100.0f);

	if (x < -zoomed_tile_width)
		return false;
	if (x > view_port.w)
		return false;
	if (y < -zoomed_tile_height)
		return false;
	return true;
}

void Camera::Move(iPoint destination, int speed)
{
	moving = true;

	this->destination = destination;
	this->speed = speed;

	y_movement = abs(destination.y - view_port.y);
	x_movement = abs(destination.x - view_port.x);
}

void Camera::MoveUp(float amount)
{
	view_port.y += floor(amount);
}

void Camera::MoveLeft(float amount)
{
	view_port.x += floor(amount);
}

void Camera::MoveDown(float amount)
{
	view_port.y -= floor(amount);
}

void Camera::MoveRight(float amount)
{
	view_port.x -= floor(amount);
}

void Camera::SetZoom(float zoom_percentage)
{
	zoom = zoom_percentage;
}

void Camera::ZoomIn(float percentage)
{
	zoom += percentage;
}

void Camera::ZoomOut(float percentage)
{
	zoom -= percentage;
}

SDL_Rect Camera::GetZoomedRect(const SDL_Rect &starting_rect) const
{
	SDL_Rect rect = starting_rect;

	rect.w = rect.w * (1.0f + zoom / 100.0f);

	rect.h = rect.h * (1.0f + zoom / 100.0f);

	rect.x = (rect.x - view_port.w / 2.0f) * (1.0f + (zoom / 100.0f)) + view_port.w / 2.0f;

	rect.y = (rect.y - view_port.h / 2.0f) * (1.0f + (zoom / 100.0f)) + view_port.h / 2.0f;

	return rect;
}

void Camera::FadeToBlack(int secs_to_black, int wait, int secs_to_light)
{
	frames_to_black = secs_to_black * FPS;
	frames_in_black = wait * FPS;
	frames_to_light = secs_to_light * FPS;
	opacity_delta = 255 / frames_to_black;
}

void Camera::Pan(Entity * entity)
{
	follow = entity;
	pan = true;
}

void Camera::UnPan()
{
	pan = false;
}

void Camera::SetCenter(iPoint pos)
{
	view_port.x = -(pos.x - view_port.w / 2);
	view_port.y = -(pos.y - view_port.h / 2);
}

void Camera::UpdateCamera()
{
	if (moving)
	{
		int total_movement = y_movement + x_movement;

		if (destination.x > view_port.x)
		{
			if ((speed * (float(x_movement) / float(total_movement))) > (destination.x - view_port.x))
			{
				view_port.x = destination.x;
				moving = false;
			}
			else
				view_port.x += speed * (float(x_movement) / float(total_movement));
		}
		else
		{
			if ((speed * (float(x_movement) / float(total_movement))) > abs(destination.x - view_port.x))
			{
				view_port.x = destination.x;
				moving = false;
			}
			else
				view_port.x -= speed * (float(x_movement) / float(total_movement));
		}

		if (destination.y > view_port.y)
		{
			if ((speed * (float(y_movement) / float(total_movement))) > (destination.y - view_port.y))
			{
				view_port.y = destination.y;
				moving = false;
			}
			else
				view_port.y += speed * (float(y_movement) / float(total_movement));
		}
		else
		{
			if ((speed * (float(y_movement) / float(total_movement))) > abs(destination.y - view_port.y))
			{
				view_port.y = destination.y;
				moving = false;
			}
			else
				view_port.y -= speed * (float(x_movement) / float(total_movement));
		}
	}


	if (frames_to_black > 0)
	{
		frames_to_black--;
		opacity -= opacity_delta;
		if (frames_to_black == 0)
			opacity_delta = 255 / frames_to_light;
	}
	else if (frames_in_black > 0)
		frames_in_black--;
	else if (frames_to_light > 0)
	{
		opacity += opacity_delta;
		frames_to_light--;
	}

	if (pan && follow != nullptr)
		SetCenter(iPoint(follow->GetX(), follow->GetY()));
}