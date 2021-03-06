#include "App.h"
#include "M_Minimap.h"
#include "M_Render.h"
#include "M_FileSystem.h"
#include "M_Textures.h"
#include "M_Map.h"
#include "M_EntityManager.h"
#include "Log.h"
#include "Entity.h"
#include <math.h>

#include "SDL\include\SDL.h"
#include "SDL_TTF\include\SDL_ttf.h"
#pragma comment( lib, "SDL_ttf/libx86/SDL2_ttf.lib" )

#define UNITS_POS_X (app->render->camera->GetSize().x * 1239 / 1366) - app->render->camera->GetPosition().x + (coords.x * ratioX)
#define UNITS_POS_Y (app->render->camera->GetSize().y * 640 / 768) - app->render->camera->GetPosition().y + (coords.y * ratioY)

M_Minimap::M_Minimap(bool startEnabled) : Module(startEnabled)
{
	name.assign("minimap");
}

//Destructor
M_Minimap::~M_Minimap()
{}

bool M_Minimap::Awake(pugi::xml_node& config)
{
	return true;
}

bool M_Minimap::CleanUp()
{
	app->tex->UnLoad(minimap_atlas);
	return true;
}

void M_Minimap::DrawMinimap() 
{
	if (this != nullptr)
	{

		float ratioX = (app->render->camera->GetSize().x * 326.0f / 1920.0f) / 8640.0f;
		float ratioY = (app->render->camera->GetSize().y * 174.0f / 1080.0f) / 4320.0f;
		std::vector<Entity*> uVector = app->entityManager->GetUnitVector();
		for (std::vector<Entity*>::iterator it = uVector.begin(); it != uVector.end(); it++) {
			iPoint coords;
			coords.x = (*it)->GetGlobalPosition().x;
			coords.y = (*it)->GetGlobalPosition().y;
			if (dynamic_cast<Unit*>(*it)->horde == true)
			{			
				app->render->Blit(minimap_atlas, UNITS_POS_X, UNITS_POS_Y, &red);
			}
			else
			{
				app->render->Blit(minimap_atlas, UNITS_POS_X, UNITS_POS_Y, &green);
			}
		}
		int quadX = (app->render->camera->GetSize().x * 1743 / 1920) - app->render->camera->GetPosition().x - (app->render->camera->GetPosition().x * ratioX);
		int quadY = (app->render->camera->GetSize().y * 900 / 1080) - app->render->camera->GetPosition().y - (app->render->camera->GetPosition().y * ratioY);
		int width = app->render->camera->GetSize().x * 32 / 1366;
		int height = app->render->camera->GetSize().y * 18 / 768;
		app->render->DrawQuad({ quadX, quadY, width, height }, 255, 255, 255, 255, false);
	}
}

void M_Minimap::DrawDebug()
{
}