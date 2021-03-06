#include "S_InGame.h"
#include "App.h"
#include "M_Map.h"
#include "M_Pathfinding.h"
#include "M_EntityManager.h"
#include "M_FogOfWar.h"
#include "M_Audio.h"
#include "M_EnemyWaves.h"
#include "Entity.h"
#include "Building.h"
#include "M_DialogueManager.h"
#include "S_Menu.h"
#include "M_MissionManager.h"
#include "M_Minimap.h"
#include "M_Input.h"
#include "M_Resources.h"
#include "M_ParticleSystem.h"
#include "S_Score.h"
#include "M_Textures.h"
#include "M_KeyBinding.h"
#include "M_Metrics.h"

S_InGame::S_InGame(bool startEnabled) : Module(startEnabled)
{
	name.assign("inGame");
	active = false;
}


S_InGame::~S_InGame()
{
}

bool S_InGame::Awake(pugi::xml_node & config)
{
	return true;
}

bool S_InGame::Start()
{
	if (active)
	{
		app->pathfinding->Enable();
		app->entityManager->LoadObjects();
		SetGUI();

		app->dialogueManager->Enable();
		app->dialogueManager->Start();

		app->missionManager->Enable();
		app->missionManager->misionTimer.Start();
		//audio
		//app->audio->PlayTheme(app->audio->thirdMission);

		BuldingCreator();//Create the buldings
		UnitsCreator();// Create the start units

		app->render->camera->SetCenter({ 100, 3560 });

		if (app->map->IsMapLoaded() == false)
		{
			if (app->map->Load("Map.tmx") == true)
			{
				int w, h;
				uchar* data = NULL;
				if (app->map->CreateWalkabilityMap(w, h, &data))
					app->pathfinding->SetMap(w, h, data);

				RELEASE_ARRAY(data);

				//app->fogOfWar->GenerateFogOfWar();
			}
		}

		app->minimap->minimap_atlas = app->tex->Load("maps/minimap.png");

		app->entityManager->PlaceObjects();

		app->resources->Start();

		app->enemyWaves->Start();

		app->audio->PlayTheme(app->audio->firstMission);

		onTutorial = true;

		app->pause = true;
	}
	return true;
}

update_status S_InGame::PreUpdate(float dt)
{
	if (active)
	{
		if (onTutorial && app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_UP)
		{
			app->gui->tutorialImage->SetVisible(false);
			onTutorial = false;
			app->pause = false;
		}

		if (app->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN && !onTutorial) 
		{
			OpenMenu(!menuOpen);
			menuOpen = !menuOpen;
		}
		
		//Create objects
		ToolCreateObjects();

		if (!app->pause)
		{
			// -------------------Move camera--------------------------------------------

			if (app->input->GetKey(app->keyBinding->cameraUp.scancode) == KEY_REPEAT)
				app->render->camera->Move(10.0, UP);
			if (app->input->GetKey(app->keyBinding->cameraLeft.scancode) == KEY_REPEAT)
				app->render->camera->Move(10.0, LEFT);
			if (app->input->GetKey(app->keyBinding->cameraDown.scancode) == KEY_REPEAT)
				app->render->camera->Move(10.0, DOWN);
			if (app->input->GetKey(app->keyBinding->cameraRight.scancode) == KEY_REPEAT)
				app->render->camera->Move(10.0, RIGHT);

			int mX, mY;
			app->input->GetMouseScreenPosition(mX, mY);

			if (mY < 5)
				app->render->camera->Move(10.0, UP);
			if (mY > app->render->camera->GetSize().y - 5)
				app->render->camera->Move(10.0, DOWN);
			if (mX < 5)
				app->render->camera->Move(10.0, LEFT);
			if (mX > app->render->camera->GetSize().x - 5)
				app->render->camera->Move(10.0, RIGHT);
		}
			// -------------------------Creators -----------------------------------

		/*	if (app->input->GetKey(SDL_SCANCODE_5) == KEY_DOWN)
			{
				int x, y;
				app->input->GetMouseMapPosition(x, y);
				app->entityManager->CreateUnit(ARCHER, nullptr, x, y);
			}
			if (app->input->GetKey(SDL_SCANCODE_6) == KEY_DOWN)
			{
				int x, y;
				app->input->GetMouseMapPosition(x, y);
				app->entityManager->CreateUnit(TARKAN_KNIGHT, nullptr, x, y);
				app->audio->PlayFx(app->entityManager->fxCreateUnit);
			}
			if (app->input->GetKey(SDL_SCANCODE_7) == KEY_DOWN)
			{
				int x, y;
				app->input->GetMouseMapPosition(x, y);
				app->entityManager->CreateUnit(VILE, nullptr, x, y);
				app->audio->PlayFx(app->entityManager->fxCreateUnit);
			}
			if (app->input->GetKey(SDL_SCANCODE_J) == KEY_DOWN)
			{
				return UPDATE_STOP;
			}*/

			//if (app->input->GetKey(SDL_SCANCODE_K) == KEY_DOWN) //Serialize
			//app->entityManager->SerializeObjects();

			/*if (app->input->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
			app->gui->SetActiveScene(name);
			if (app->input->GetKey(SDL_SCANCODE_F10) == KEY_DOWN)
			GoToMenu();
			if (app->input->GetKey(SDL_SCANCODE_F11) == KEY_DOWN)
			app->gui->SetActiveScene("\0");*/

			// --------------------------------scenes changes----------------------------
			//	M_INTRO,M_TOWNATTACK,M_TOWNREPAIR, M_WAVES,	M_BOSS,	M_VICTORY,	M_DEFEAT,M_STANDBY
		
	}	
	return UPDATE_CONTINUE;
}

update_status S_InGame::Update(float dt)
{
	if (active)
	{
		if (!app->pause)
		{
			if (app->input->GetKey(SDL_SCANCODE_S) == KEY_UP)
			{
				app->SaveGame("test.xml");
			}
			if (app->input->GetKey(SDL_SCANCODE_L) == KEY_UP)
			{
				app->LoadGame("test.xml");
			}

		}
	}
	return UPDATE_CONTINUE;
}

update_status S_InGame::PostUpdate(float dt)
{
	if (active)
	{

	}
	return UPDATE_CONTINUE;
}

void S_InGame::Draw()
{
	if (active)
	{
		
	}
}

void S_InGame::DrawDebug()
{
	if (active)
	{
		for (auto node : path)
		{
			iPoint point;
			point = app->map->MapToWorld(node.x, node.y);
			app->render->DrawQuad({ point.x - 5, point.y - 5, 10, 10 }, 255, 255, 0, 255);
		}
	}
}

void S_InGame::GuiEvent(GUIElement* element, int64_t event)
{
	if (event & MOUSE_LCLICK_UP)
	{
		if (event & OPEN_MENU)
		{
			OpenMenu(!menuOpen);
			menuOpen = !menuOpen;
			app->audio->PlayFx(app->gui->fxSelect);
		}
		if (event & OPEN_RESOURCE_MANAGER)
		{
			OpenResources(!resourceManager);
			resourceManager = !resourceManager;
			app->audio->PlayFx(app->gui->fxSelect);
		}
		if (event & CLOSE_APP)
		{
			app->quit = true;
		}
		if (event & LOAD_GAME)
		{
			element->SetLClicked(false);
			OpenMenu(!menuOpen);
			menuOpen = !menuOpen;
			app->LoadGame("test.xml");
			app->audio->PlayFx(app->gui->fxSelect);
		}
		if (event & SAVE_GAME)
		{
			element->SetLClicked(false);
			OpenMenu(!menuOpen);
			menuOpen = !menuOpen;
			app->SaveGame("test.xml");
			app->audio->PlayFx(app->gui->fxSelect);	
		}
		if (event & START_GAME)
		{
			element->SetLClicked(false);
			OpenMenu(!menuOpen);
			menuOpen = !menuOpen;
			app->audio->PlayFx(app->gui->fxSelect);
		}
		if (event & ADD_MINER)//AKA Return to Menu
		{
			element->SetLClicked(false);
			GoToMenu();
		}
	}
}

void S_InGame::GoToMenu()
{
	active = false;
	app->menu->Start();
	app->dialogueManager->CleanUp();
	app->dialogueManager->Disable();
	app->entityManager->CleanUp();
	app->audio->CleanData();
	app->missionManager->CleanUp();
	app->missionManager->Disable();
	app->minimap->CleanUp();
	app->particleSystem->DestroyParticles();
	app->enemyWaves->CleanUp();
	app->metrics->CleanUp();

	app->entityManager->Awake(pugi::xml_node(nullptr));
	app->entityManager->Start();
}

void S_InGame::GoToScore()
{
	active = false;
	app->score->active = true;
	app->score->Start();
	app->score->bg->SetVisible(true);
	app->dialogueManager->CleanUp();
	app->dialogueManager->Disable();
	app->gui->SetActiveScene("score");
	app->entityManager->CleanUp();
	app->audio->CleanData();
	app->missionManager->CleanUp();
	app->missionManager->Disable();
	app->minimap->CleanUp();
	app->particleSystem->DestroyParticles();
	app->enemyWaves->CleanUp();
	app->metrics->CleanUp();

	app->entityManager->Awake(pugi::xml_node(nullptr));
	app->entityManager->Start();
}

void S_InGame::OpenMenu(bool visible)
{
	app->gui->FindElement(app->gui->guiList, "MenuWindow")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Resume")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Load")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Save")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Start")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Exit")->SetVisible(visible);

	app->gui->FindElement(app->gui->guiList, "MenuWindow")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Resume")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Load")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Save")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Start")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Exit")->SetInteractive(visible);

	app->pause = visible;

	if(visible == true)
	{
		app->missionManager->misionTimer.Pause();
	}
	else
	{
		app->missionManager->misionTimer.UnPause();
	}
}

void S_InGame::OpenResources(bool visible)
{
	app->gui->FindElement(app->gui->guiList, "ResourceWindow")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_TotalVillager")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_TotalVillager_n")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_UnployedVillager")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_UnployedVillager_n")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Miners")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Miners_n")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Miners_plus")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Miners_minus")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Farmers")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Farmers_n")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Farmers_plus")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Farmers_minus")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Lumberjacks")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Lumberjacks_n")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Lumberjacks_plus")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Lumberjacks_minus")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Repairmen")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Repairmen_n")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Repairmen_plus")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Repairmen_minus")->SetVisible(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_TotalVillager__")->SetVisible(visible);

	app->gui->FindElement(app->gui->guiList, "ResourceWindow")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_TotalVillager")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_TotalVillager_n")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_UnployedVillager")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_UnployedVillager_n")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Miners")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Miners_n")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Miners_plus")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Miners_minus")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Farmers")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Farmers_n")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Farmers_plus")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Farmers_minus")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Lumberjacks")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Lumberjacks_n")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Lumberjacks_plus")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Lumberjacks_minus")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Repairmen")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Repairmen_n")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Repairmen_plus")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Repairmen_minus")->SetInteractive(visible);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_TotalVillager__")->SetInteractive(visible);
}

void S_InGame::SetGUI()
{
	app->gui->FindElement(app->gui->guiList, "MenuWindow")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Resume")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Load")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Save")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Options")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Start")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Exit")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "RepairButtonBarrack")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "RepairButtonStable")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "RepairButtonArchery")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "SamuraiCreatorButton")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "TarkanCreatorButton")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "ArcherCreatorButton")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "VillagerCreatorButton")->SetVisible(false);

	//----resource window
	app->gui->FindElement(app->gui->guiList, "ResourceWindow")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_TotalVillager")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_TotalVillager_n")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_UnployedVillager")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_UnployedVillager_n")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Miners")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Miners_n")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Miners_plus")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Miners_minus")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Farmers")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Farmers_n")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Farmers_plus")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Farmers_minus")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Lumberjacks")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Lumberjacks_n")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Lumberjacks_plus")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Lumberjacks_minus")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Repairmen")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Repairmen_n")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Repairmen_plus")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Repairmen_minus")->SetVisible(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_TotalVillager__")->SetVisible(false);
	//----
	app->gui->FindElement(app->gui->guiList, "MenuWindow")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Resume")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Load")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Save")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Start")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Exit")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "MenuButtonInGame_Options")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "RepairButtonBarrack")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "RepairButtonStable")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "RepairButtonArchery")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "SamuraiCreatorButton")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "TarkanCreatorButton")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "ArcherCreatorButton")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "VillagerCreatorButton")->SetInteractive(false);

	//----resource window
	app->gui->FindElement(app->gui->guiList, "ResourceWindow")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_TotalVillager")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_TotalVillager_n")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_UnployedVillager")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_UnployedVillager_n")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Miners")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Miners_n")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Miners_plus")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Miners_minus")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Farmers")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Farmers_n")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Farmers_plus")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Farmers_minus")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Lumberjacks")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Lumberjacks_n")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Lumberjacks_plus")->SetInteractive(false); 
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Lumberjacks_minus")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Repairmen")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Repairmen_n")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Repairmen_plus")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_Repairmen_minus")->SetInteractive(false);
	app->gui->FindElement(app->gui->guiList, "label_ResourceWindow_TotalVillager__")->SetInteractive(false);

	// RepairButton - PlusButton - MinusButton

	//Tutorial
	if (app->gui->tutorialImage == nullptr)
	{
		app->gui->tutorialImage = app->gui->CreateImage({ 0,0,(int)(1920 * app->gui->GetScaleX()),(int)(1080 * app->gui->GetScaleY()) }, { 0, 0, 1920, 1080 }, "tutorial");

		dynamic_cast<GUIImage*>(app->gui->tutorialImage)->SetAtlas(app->tex->Load("gui/tutorial.png"));
		app->gui->tutorialImage->SetVisible(true);
	}
	else
	{
		app->gui->tutorialImage->SetVisible(true);
	}
}

// ----------------------- CREATE -----------------------------------
void S_InGame::BuldingCreator()
{	
	//3254 1695
	app->entityManager->CreateBuilding(BUILD_STABLES, iPoint(9, 72), nullptr, -3254, 1695);
	app->entityManager->CreateBuilding(BUILD_ARCHERY, iPoint(24, 74), nullptr, -2650, 2072);
	app->entityManager->CreateBuilding(BUILD_BARRACK, iPoint(19, 61), nullptr, -2120, 1657);
	app->entityManager->CreateBuilding(BUILD_BARRACK, iPoint(23, 61), nullptr, -1920, 1757);
	app->entityManager->CreateBuilding(BUILD_TOWNCENTER, iPoint(29, 72), nullptr, -2160, 2067);

	app->entityManager->CreateBuilding(BUILD_PORTAL, iPoint(5, 80), nullptr, -3782, 1851);
	app->entityManager->CreateBuilding(BUILD_PORTAL, iPoint(4, 51), nullptr, -2390, 1107);
	app->entityManager->CreateBuilding(BUILD_PORTAL, iPoint(27, 31), nullptr, -134, 1179);
	app->entityManager->CreateBuilding(BUILD_PORTAL, iPoint(57, 54), nullptr, 202, 2497);
}

void S_InGame::UnitsCreator()
{
	//Boss
	app->entityManager->CreateUnit(DIABLO, nullptr, -2190, 2277);

	//Enemies
	app->entityManager->CreateUnit(VILE, nullptr, -950, 3498);//
	app->entityManager->CreateUnit(VILE, nullptr, -1000, 3498);//
	app->entityManager->CreateUnit(VILE, nullptr, -900, 3398);//
	app->entityManager->CreateUnit(VILE, nullptr, -1000, 2898);//
	app->entityManager->CreateUnit(VILE, nullptr, -950, 2898);//
	app->entityManager->CreateUnit(VILE, nullptr, -950, 2798);//
	app->entityManager->CreateUnit(VILE, nullptr, -900, 2898);//
	app->entityManager->CreateUnit(VILE, nullptr, -950, 2598);//
	app->entityManager->CreateUnit(VILE, nullptr, -1000, 2598);//
	app->entityManager->CreateUnit(VILE, nullptr, -650, 2208);//
	app->entityManager->CreateUnit(VILE, nullptr, -600, 2198);//
	app->entityManager->CreateUnit(VILE, nullptr, -620, 2158);//

	//Allies
	app->entityManager->CreateUnit(ARCHER, nullptr, 287, 3698);//
	app->entityManager->CreateUnit(ARCHER, nullptr, 357, 3658);//
	app->entityManager->CreateUnit(ARCHER, nullptr, 287, 3698);//
	app->entityManager->CreateUnit(ARCHER, nullptr, 257, 3658);//
	app->entityManager->CreateUnit(ARCHER, nullptr, 287, 3658);//
	app->entityManager->CreateUnit(ARCHER, nullptr, 227, 3658);//
	app->entityManager->CreateUnit(SAMURAI, nullptr, 257, 3658);//
	app->entityManager->CreateUnit(SAMURAI, nullptr, 257, 3668);//
	app->entityManager->CreateUnit(SAMURAI, nullptr, 257, 3658);//
	app->entityManager->CreateUnit(SAMURAI, nullptr, 287, 3658);//
	app->entityManager->CreateUnit(SAMURAI, nullptr, 257, 3658);//
	app->entityManager->CreateUnit(SAMURAI, nullptr, 307, 3658);//
	app->entityManager->CreateUnit(SAMURAI, nullptr, 257, 3668);//
	app->entityManager->CreateUnit(SAMURAI, nullptr, 257, 3658);//
	app->entityManager->CreateUnit(SAMURAI, nullptr, 287, 3658);//
	app->entityManager->CreateUnit(SAMURAI, nullptr, 257, 3658);//
	app->entityManager->CreateUnit(SAMURAI, nullptr, 307, 3658);//
}

void S_InGame::ToolCreateObjects()
{
	 //trees 
/*	if (app->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN) {  //ARBOL 1
		int posx, posY;
		app->input->GetMouseMapPosition(posx, posY);
		app->entityManager->CreateObject(TREES1, nullptr, posx, posY);
	}
	if (app->input->GetKey(SDL_SCANCODE_2) == KEY_DOWN) {  //ARBOL 2
		int posx, posY;
		app->input->GetMouseMapPosition(posx, posY);
		app->entityManager->CreateObject(TREES2, nullptr, posx, posY);
	}
	if (app->input->GetKey(SDL_SCANCODE_3) == KEY_DOWN) {  //ARBOL 3
		int posx, posY;
		app->input->GetMouseMapPosition(posx, posY);
		app->entityManager->CreateObject(TREES3, nullptr, posx, posY);
	}
	if (app->input->GetKey(SDL_SCANCODE_4) == KEY_DOWN) {  //ARBOL 4
		int posx, posY;
		app->input->GetMouseMapPosition(posx, posY);
		app->entityManager->CreateObject(TREES4, nullptr, posx, posY);
	}
	if (app->input->GetKey(SDL_SCANCODE_5) == KEY_DOWN) {  //ARBOL 1
		int posx, posY;
		app->input->GetMouseMapPosition(posx, posY);
		app->entityManager->CreateObject(TREES5, nullptr, posx, posY);
	}
	if (app->input->GetKey(SDL_SCANCODE_6) == KEY_DOWN) {  //ARBOL 2
		int posx, posY;
		app->input->GetMouseMapPosition(posx, posY);
		app->entityManager->CreateObject(TREES6, nullptr, posx, posY);
	}
	if (app->input->GetKey(SDL_SCANCODE_7) == KEY_DOWN) {  //ARBOL 3
		int posx, posY;
		app->input->GetMouseMapPosition(posx, posY);
		app->entityManager->CreateUnit(HELL_WITCH, nullptr, posx, posY);//1

	}
	if (app->input->GetKey(SDL_SCANCODE_8) == KEY_DOWN) {  //ARBOL 4
		int posx, posY;
		app->input->GetMouseMapPosition(posx, posY);
		app->entityManager->CreateObject(TREES8, nullptr, posx, posY);
	}*/

	////bamboo

	//if (app->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN) {  //BAMBOO 1
	//	int posx, posY;
	//	app->input->GetMouseMapPosition(posx, posY);
	//	app->entityManager->CreateObject(BAMBOO1, nullptr, posx, posY);
	//}
	//if (app->input->GetKey(SDL_SCANCODE_2) == KEY_DOWN) {  //BAMBOO 2
	//	int posx, posY;
	//	app->input->GetMouseMapPosition(posx, posY);
	//	app->entityManager->CreateObject(BAMBOO2, nullptr, posx, posY);
	//}
	//if (app->input->GetKey(SDL_SCANCODE_3) == KEY_DOWN) {  //BAMBOO 3
	//	int posx, posY;
	//	app->input->GetMouseMapPosition(posx, posY);
	//	app->entityManager->CreateObject(BAMBOO3, nullptr, posx, posY);
	//}
	//if (app->input->GetKey(SDL_SCANCODE_4) == KEY_DOWN) {  //BAMBOO 3
	//	int posx, posY;
	//	app->input->GetMouseMapPosition(posx, posY);
	//	app->entityManager->CreateObject(BAMBOO4, nullptr, posx, posY);
	//}

	////Houses

	//if (app->input->GetKey(SDL_SCANCODE_6) == KEY_DOWN) {  //HOUSE 1
	//	int posx, posY;
	//	app->input->GetMouseMapPosition(posx, posY);
	//	app->entityManager->CreateObject(HOUSE1, nullptr, posx, posY);
	//}
	//if (app->input->GetKey(SDL_SCANCODE_7) == KEY_DOWN) {  //HOUSE 2
	//	int posx, posY;
	//	app->input->GetMouseMapPosition(posx, posY);
	//	app->entityManager->CreateObject(HOUSE2, nullptr, posx, posY);
	//}
	//if (app->input->GetKey(SDL_SCANCODE_8) == KEY_DOWN) {  //HOUSE 3
	//	int posx, posY;
	//	app->input->GetMouseMapPosition(posx, posY);
	//	app->entityManager->CreateObject(HOUSE3, nullptr, posx, posY);
	//}
	

	//buldings

	//if (app->input->GetKey(SDL_SCANCODE_8) == KEY_DOWN) {  //BAMBOO 1
	//	int posx, posY;
	//	app->input->GetMouseMapPosition(posx, posY);
	//	app->entityManager->CreateObject(MONASTERY, nullptr, posx, posY);
	//}
	//if (app->input->GetKey(SDL_SCANCODE_7) == KEY_DOWN) {  //BAMBOO 2
	//	int posx, posY;
	//	app->input->GetMouseMapPosition(posx, posY);
	//	app->entityManager->CreateObject(UNIVERSITY, nullptr, posx, posY);
	//}
}
