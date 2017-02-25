#ifndef __M_ENTITY_MANAGER_H__
#define __M_ENTITY_MANAGER_H__

#include "Module.h"

class Entity;

class M_EntityManager : public Module
{
public:
	M_EntityManager(bool startEnabled = true);
	virtual ~M_EntityManager();

	bool Awake(pugi::xml_node&)override;
	bool Start()override;
	bool PreUpdate()override;
	bool Update(float dt)override;
	bool PostUpdate()override;
	bool CleanUp()override;


	Entity* CreateEntity(Entity* parent = nullptr, int posX = 0, int posY = 0, int rectX = 1, int rectY = 1);

	Entity* GetSceneRoot()const;
	Entity* FindEntity(); //TODO: Used if UID are in use

	void Draw();
	void DrawDebug()override;

	void InsertEntityToTree(Entity* et);
	void EraseEntityFromTree(Entity* et);

	void LoadScene(); //TODO: Lot of work before using this
	void SaveScene(); //TODO: Lot of work before using this

private:
	void RemoveFlagged();
	bool SaveSceneNow();
	bool LoadSceneNow();

public:
	bool showQauds = false;
	//JQuadTree* sceneTree = nullptr;

private:
	Entity* root = nullptr;
	bool mustSaveScene = false;
	bool mustLoadScene = false;
};

#endif // !__M_ENTITY_MANAGER_H__