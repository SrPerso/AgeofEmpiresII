#include "M_EntityManager.h"
#include "Entity.h"

#include "App.h"
#include "Log.h"

//TMP
#include "M_Input.h"
#include "SDL\include\SDL.h"
#include "M_Textures.h"

/**
	Contructor: Creates the module that manages all entities.

	Parameters: Boolean. Module creates enabled or not.
*/
M_EntityManager::M_EntityManager(bool startEnabled) : Module(startEnabled)
{
	LOG("Entity manager: Creation.");
	name.assign("entity_manager");
}

/**
	Destructor
*/
M_EntityManager::~M_EntityManager()
{
	LOG("Entity manager: Destroying.");
}

/**
	Awake: Called before the first frame. Creates the root entity of the scene.

	Return:
		-True if succes.
		-False if could not create the root.

	Parameters: xml_node with the config file section.
*/
bool M_EntityManager::Awake(pugi::xml_node&)
{
	LOG("Entity manager: Awake.");
	bool ret = true;

	root = new Entity(nullptr);
	if (!root)
	{
		ret = false;
		LOG("ERROR: Could not create scene root entity.");
	}

	return ret;
}

/**
	Start: Called before first frame if module is active. Sets the base scene.

	Return:
		-True if succes.
		-False if any error.

	Parameters: Void.
*/
bool M_EntityManager::Start()
{
	LOG("Entity manager: Start.");
	bool ret = true;

	//TMP
	textTexture = app->tex->Load("textures/test.png");

	et = CreateEntity(nullptr, 300, 100);
	et->SetTexture(textTexture);
	//et2 = CreateEntity(et, 50, 50);
	//et2->SetTexture(textTexture, GB_Rectangle<int>(0, 0, 100, 100));
	archer = CreateUnit(CAVALRY_ARCHER, nullptr, 1000, 300);

	return ret;
}

/**
	PreUpdate: Called every frame. Does logic calcs on all entities.

	Return: 
		-UPDATE_CONTINUE if all is correct.
		-UPDATE_STOP if the engine has to stop correctly.
		-UPDATE_ERROR if there's any error.

		Parameters: Float delta time.
*/
update_status M_EntityManager::PreUpdate(float dt)
{
	update_status ret = UPDATE_CONTINUE;

	RemoveFlagged();

	if (root)
	{
		root->RecCalcTransform({ 0, 0 });
		root->RecCalcBox();
	}
	else
	{
		LOG("ERROR: Invalid root, is NULL.");
		ret = UPDATE_ERROR;
	}

	if (mustSaveScene && ret == UPDATE_CONTINUE)
	{
		SaveSceneNow();
		mustSaveScene = false;
	}

	if (mustLoadScene && ret == UPDATE_CONTINUE)
	{
		LoadSceneNow();
		mustLoadScene = false;
	}

	//TMP
	if (app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT)
	{
		iPoint pos;
		app->input->GetMousePosition(pos.x, pos.y);
		et->SetLocalPosition(pos);
	}

	if (app->input->GetKey(SDL_SCANCODE_M) == KEY_REPEAT)
	{
		iPoint pos;
		app->input->GetMousePosition(pos.x, pos.y);
		//et2->SetLocalPosition(pos);
		et2->SetGlobalPosition(pos);
	}

	return ret;
}

/**
	Update: Called every frame. Calls all entities updates.
	
	Return:
		-UPDATE_CONTINUE if all is correct.
		-UPDATE_STOP if the engine has to stop correctly.
		-UPDATE_ERROR if there's any error.
	
	Parameters: Float delta time.
*/
update_status M_EntityManager::Update(float dt)
{
	update_status ret = UPDATE_CONTINUE;

	if (root)
	{
		//TODO: Only if play mode??
		root->OnUpdate(dt); //TODO: Test with tree??? If not drawn why update?? Add static and dynamic entities??
	}
	else
	{
		LOG("ERROR: Invalid root, is NULL.");
		ret = UPDATE_ERROR;
	}

	return ret;
}

/**
	PostUpdate: Called every frame.
	
	Return:
		-UPDATE_CONTINUE if all is correct.
		-UPDATE_STOP if the engine has to stop correctly.
		-UPDATE_ERROR if there's any error.
	
	Parameters: Float delta time.
*/
update_status M_EntityManager::PostUpdate(float dt)
{
	update_status ret = UPDATE_CONTINUE;

	return ret;
}

/**
	CleanUp: Called At the end of the execution. Cleans all the scene.
	
	Return:
		-True if succes.
		-False if any error.
	
	Parameters: Void.
*/
bool M_EntityManager::CleanUp()
{
	LOG("Entity manager: CleanUp.");
	bool ret = true;

	root->Remove();
	//TODO: Autosave scene

	RELEASE(root);

	return ret;
}

/**
	CreateEntity: Creates a new entity and adds it to the scene.

	Return:
		-Entity pointer if succes.
		-Nullptr if failure.

	Parameters:
		-Entity* if force a parent. Root by default.
		-posX, posY to force a position. 0,0 by default.
		-rectX, rectY to force an enclosing box size. 1,1 by default.
*/
Entity* M_EntityManager::CreateEntity(Entity* parent, int posX, int posY, int rectX, int rectY)
{
	Entity* ret = nullptr; //First set to nullptr to assure we can check for errors later.
	ret = new Entity(parent);

	if (parent)
		parent->AddChild(ret);
	else
		root->AddChild(ret);

	if (ret)
	{
		ret->SetLocalPosition(posX, posY);
		ret->SetEnclosingBox(posX, posY, rectX, rectY);
	}
	else
	{
		LOG("ERROR: Could not create a new entity.");
	}

	return ret;
}

/**
	CreateUnit: Creates a new unit and adds it to scene.

	Return:
		-Entity pointer if succes.
		-Nullptr if failure.

	Parameters:
		-unit_type to determine the unit type.
		-Entity* if force a parent. Root by default.
		-posX, posY to force a position. 0,0 by default.
		-rectX, rectY to force an enclosing box size. 1,1 by default.
*/
Entity* M_EntityManager::CreateUnit(unit_type type, Entity* parent, int posX, int posY, int rectX, int rectY)
{
	Entity* ret = (Entity*) new Unit(type, parent);

	if (parent)
		parent->AddChild(ret);
	else
		root->AddChild(ret);

	if (ret)
	{
		ret->SetLocalPosition(posX, posY);
		ret->SetEnclosingBox(posX, posY, rectX, rectY);
	}
	else
	{
		LOG("ERROR: Could not create a new unit.");
	}

	return ret;
}

/**
	GetSceneRoot: Return the root entity of the scene.

	Return:
		-Constant entity.

	Parameters:
		-Void.
*/
Entity* M_EntityManager::GetSceneRoot()const
{
	return root;
}

Entity* M_EntityManager::FindEntity()
{
	return nullptr; //TODO
}

/**
	Draw: Collect all enitities that must be drawn on screen by testing the camera with the quadtree(TODO) and fill a vector with them.

	Return: 
		-Void.

	Parameters:
		-Entiti* vector by reference that will be filled.
*/
void M_EntityManager::Draw(std::vector<Entity*>& entitiesToDraw)
{
	if (root)
	{
		//TODO: Must check with quadtree
		for (std::vector<Entity*>::iterator it = root->childs.begin(); it != root->childs.end(); ++it)
		{
			RecColectEntitiesToDraw(entitiesToDraw, (*it));
		}
	}
	else
	{
		LOG("ERROR: Invalid root, is NULL.");
	}
}

/**
	DrawDebug: Called every frame if debug mode is enabled. Calls all active entities DrawDebug.

	Return: 
		-Void.

	Parameters:
		-Void.
*/
void M_EntityManager::DrawDebug()
{
	if (root)
	{
		for (std::vector<Entity*>::iterator it = root->childs.begin(); it != root->childs.end(); ++it)
		{
			if ((*it) && (*it)->IsActive())
				(*it)->DrawDebug();
		}
	}
	else
	{
		LOG("ERROR: Invalid root, is NULL.");
	}
}

/**
	InsertEntityToTree: Adds an entity to the scene quadtree.

	Return: 
		-Void.

	Parameters:
		-Entity to add.
*/
void M_EntityManager::InsertEntityToTree(Entity* et)
{

}

/**
	EraseEntityFromTree: Remove an entity to the scene quadtree.

	Return:
		-Void.

	Parameters:
		-Entity to remove.
*/
void M_EntityManager::EraseEntityFromTree(Entity* et)
{

}

/**
	LoadScene: Mark to load an scene on next frame.
*/
void M_EntityManager::LoadScene()
{
	mustLoadScene = true;
}

/**
	SaveScene: Mark to save an scene on next frame.
*/
void M_EntityManager::SaveScene()
{
	mustSaveScene = true;
}

/**
	RemoveFlagged: Iterate all marked to remove entities and deletes them. Must be called on a save moment.
*/
void M_EntityManager::RemoveFlagged()
{
	if (root)
	{
		if (root->removeFlag)
		{
			for (uint i = 0; i < root->childs.size(); ++i)
			{
				root->childs[i]->Remove();
			}
			root->removeFlag = false;
		}

		if (root->RecRemoveFlagged())
		{
			//TODO: Send event
		}
	}
	else
	{
		LOG("ERROR: Invalid root, is NULL.");
	}
}

/**
	RecColectEntitiesToDraw: Recursively iterate all entities and collects them to later draw.

	Return: 
		-Void.

	Parameters:
		-Enitiy vector by reference to fill.
		-Current entity iterating.
*/
void M_EntityManager::RecColectEntitiesToDraw(std::vector<Entity*>& entitiesToDraw, Entity* et)
{
	if (et != nullptr)
	{
		if(et->HasTexture())
			entitiesToDraw.push_back(et);

		for (std::vector<Entity*>::iterator it = et->childs.begin(); it != et->childs.end(); ++it)
		{
			RecColectEntitiesToDraw(entitiesToDraw, (*it));
		}
	}
}

/**
	LoadSceneNow: Actually load a scene in a save moment.

	Return:
		-True if succes.
		-False if failure.
*/
bool M_EntityManager::LoadSceneNow()
{
	return false;
}

/**
	SaveSceneNow: Actually save a scene in a save moment.

	Return:
		-True if succes.
		-False if failure.
*/
bool M_EntityManager::SaveSceneNow()
{
	return false;
}
