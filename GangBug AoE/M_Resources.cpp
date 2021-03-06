#include "App.h"
#include "M_Resources.h"
#include "M_DialogueManager.h"
#include "M_EntityManager.h"
#include "M_MissionManager.h"
#include "Entity.h"
#include "Building.h"
#include "j1Timer.h"
#include "Log.h"
#include "M_Audio.h"
#include "M_GUI.h"
#include "S_InGame.h"
#include "M_Metrics.h"

M_Resources::M_Resources(bool startEnabled) : Module(startEnabled)
{
	name.assign("resources");
}

M_Resources::~M_Resources()
{
}

bool M_Resources::Start()
{
	LOG("Resources: Start.");
	bool ret = true;

	food = 150;
	wood = 150;
	gold = 150;

	miners = 0;
	lumberjacks = 0;
	farmers = 0;
	constructors = 0;
	totalVillagers = 3;
	unemployedVillagers = 3;
	totalUnits = 0;

	//EASY MODE
	if (app->missionManager->getHardModeStatus() == false)
	{
		foodAmount = 8;
		woodAmount = 8;
		goldAmount = 8;
	}
	//HARD MODE
	else if (app->missionManager->getHardModeStatus() == true)
	{
		foodAmount = 6;
		woodAmount = 6;
		goldAmount = 6;
	}

	return ret;
}

//Get current amount of each resource
uint M_Resources::GetCurrentFood()
{
	return food;
}
uint M_Resources::GetCurrentWood()
{
	return wood;
}
uint M_Resources::GetCurrentGold()
{
	return gold;
}

//Add certain amount to existent resources
void M_Resources::AddFood(int amount)
{
	food += amount;
}
void M_Resources::AddWood(int amount)
{
	wood += amount;
}
void M_Resources::AddGold(int amount)
{
	gold += amount;
}

//Substracts certain amount to existent resources
void M_Resources::SubstractFood(int amount)
{
	food -= amount;
}
void M_Resources::SubstractWood(int amount)
{
	wood -= amount;
}
void M_Resources::SubstractGold(int amount)
{
	gold -= amount;
}

//Set existent resources to a certain amount
void M_Resources::SetCurrentFood(int amount)
{
	food = amount;
}
void M_Resources::SetCurrentWood(int amount)
{
	wood = amount;
}
void M_Resources::SetCurrentGold(int amount)
{
	gold = amount;
}
void M_Resources::SetCurrentResources(int amount)
{
	food += amount;
	wood += amount;
	gold += amount;
}

update_status M_Resources::Update(float dt)
{
	update_status ret = UPDATE_CONTINUE;

	//Maybe we could use dt instead of this timer
	if (!app->pause && app->inGame->active == true)
	{
		//Resources update
		if (updateResources.ReadSec() > UPDATE_TIMER)
		{
			if (GetCurrentFood() < MAX_RESOURCES)
			{
				AddFood(foodAmount*farmers);
				app->metrics->AddTotalFood(foodAmount*(farmers*VILLAGERS_REDUCTION));
			}
			if (GetCurrentWood() < MAX_RESOURCES)
			{
				app->metrics->AddTotalWood(woodAmount*(lumberjacks*VILLAGERS_REDUCTION));
				AddWood(woodAmount*lumberjacks);
			}
			if (GetCurrentGold() < MAX_RESOURCES)
			{
				app->metrics->AddTotalGold(goldAmount*(miners*VILLAGERS_REDUCTION));
				AddGold(goldAmount*miners);
			}
			std::vector<Entity*> buildVector = app->entityManager->GetBuildingVector();
			for (std::vector<Entity*>::iterator it = buildVector.begin(); it != buildVector.end(); it++)
			{
				if ((*it) != nullptr)
				{
					dynamic_cast<Building*>((*it))->Repair(REPAIR_AMOUNT * constructors);
					app->metrics->AddTotalRepaired(REPAIR_AMOUNT * constructors);
				}
			}
			updateResources.Start();
		}
		if (app->input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN)
		{
			AddWood(100);
			AddFood(100);
			AddGold(100);
		}
	}
	return ret;
}

void M_Resources::DrawDebug()
{
}

uint M_Resources::GetConstructors()
{
	return uint(constructors);
}

uint M_Resources::GetVillagers()
{
	return uint(unemployedVillagers);
}

uint M_Resources::GetTotalVillagers()
{
	return uint(totalVillagers);
}

uint M_Resources::GetMiners()
{
	return uint(miners);
}

uint M_Resources::GetLumberjacks()
{
	return uint(lumberjacks);
}

uint M_Resources::GetFarmers()
{
	return uint(farmers);
}

uint M_Resources::GetTotalUnits()
{
	return uint(totalUnits);
}

void M_Resources::AddConstructors()
{
	if (unemployedVillagers > 0)
	{
		unemployedVillagers--;
		constructors++;
	}
}

void M_Resources::AddVillager()
{
	if (MAX_VILLAGERS >= totalVillagers)
	{
		totalVillagers++;
		unemployedVillagers++;
	}
}

void M_Resources::AddMiners()
{
	if (unemployedVillagers > 0)
	{
		unemployedVillagers--;
		miners++;
	}
}

void M_Resources::AddLumberjack()
{
	if (unemployedVillagers > 0) 
	{
		unemployedVillagers--;
		lumberjacks++;
	}
}

void M_Resources::AddFarmers()
{
	if (unemployedVillagers > 0) 
	{
		unemployedVillagers--;
		farmers++;
	}
}

void M_Resources::RemoveConstructors()
{
	if (constructors > 0) 
	{
		constructors--;
		unemployedVillagers++;
	}
}

void M_Resources::RemoveMiners()
{
	if (miners > 0)
	{
		miners--;
		unemployedVillagers++;
	}

}

void M_Resources::RemoveLumberjacks()
{
	if (lumberjacks > 0)
	{
		lumberjacks--;
		unemployedVillagers++;
	}
}

void M_Resources::RemoveFarmers()
{
	if (farmers > 0) 
	{
		farmers--;
		unemployedVillagers++;
	}
}

void M_Resources::GuiEvent(GUIElement * element, int64_t event)
{
	if (event & MOUSE_LCLICK_UP)
	{
		if (event & ADD_MINER)
		{
			app->audio->PlayFx(app->gui->fxSelect);
			AddMiners();
		}
		if (event & ADD_CONSTRUCTOR)
		{
			app->audio->PlayFx(app->gui->fxSelect);
			AddConstructors();
		}
		if (event & ADD_LUMBERJACK)
		{
			app->audio->PlayFx(app->gui->fxSelect);
			AddLumberjack();
		}
		if (event & ADD_FARMER)
		{
			app->audio->PlayFx(app->gui->fxSelect);
			AddFarmers();
		}
		if (event & REMOVE_MINER)
		{
			app->audio->PlayFx(app->gui->fxSelect);
			RemoveMiners();
		}
		if (event & REMOVE_CONSTRUCTOR)
		{
			app->audio->PlayFx(app->gui->fxSelect);
			RemoveConstructors();
		}
		if (event & REMOVE_LUMBERJACK)
		{
			app->audio->PlayFx(app->gui->fxSelect);
			RemoveLumberjacks();
		}
		if (event & REMOVE_FARMER)
		{
			app->audio->PlayFx(app->gui->fxSelect);
			RemoveFarmers();
		}
	}
}

bool M_Resources::Load(pugi::xml_node& node)
{
	pugi::xml_node loadNode = node.child("resourcesState");

	pugi::xml_node resourcesNode = loadNode.child("realResources");

	foodAmount = resourcesNode.attribute("food").as_int();
	woodAmount = resourcesNode.attribute("wood").as_int();
	goldAmount = resourcesNode.attribute("gold").as_int();

	pugi::xml_node villagersNode = loadNode.child("villagers");

	miners = villagersNode.attribute("miners").as_int();
	lumberjacks = villagersNode.attribute("lumberjacks").as_int();
	farmers = villagersNode.attribute("farmers").as_int();
	constructors = villagersNode.attribute("constructors").as_int();
	totalVillagers = villagersNode.attribute("totalVillagers").as_int();
	unemployedVillagers = villagersNode.attribute("unemployedVillagers").as_int();
	totalUnits = villagersNode.attribute("totalUnits").as_int();

	return true;
}
bool M_Resources::Save(pugi::xml_node& node) const
{
	pugi::xml_node saveNode = node.append_child("resourcesState");

	pugi::xml_node resourcesNode = saveNode.append_child("realResources");

	resourcesNode.append_attribute("food") = food;
	resourcesNode.append_attribute("wood") = wood;
	resourcesNode.append_attribute("gold") = gold;

	pugi::xml_node villagersNode = saveNode.append_child("villagers");

	villagersNode.append_attribute("miners") = miners;
	villagersNode.append_attribute("lumberjacks") = lumberjacks;
	villagersNode.append_attribute("farmers") = farmers;
	villagersNode.append_attribute("constructors") = constructors;
	villagersNode.append_attribute("totalVillagers") = totalVillagers;
	villagersNode.append_attribute("unemployedVillagers") = unemployedVillagers;
	villagersNode.append_attribute("totalUnits") = totalUnits;

	return true;
}