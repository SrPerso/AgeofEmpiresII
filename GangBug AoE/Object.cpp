#include "Object.h"
#include "M_EntityManager.h"
#include "App.h"
#include "M_Input.h"
#include "M_Textures.h"
#include "M_Render.h"

Object::Object(object_type objectType, Entity * parent) : Entity(ENTITY_OBJECT, parent), objectType(objectType)
{
	entityTexture = app->tex->objectTexture;
	app->entityManager->getObjectRect(objectType);
	objectTypee = int(objectType);
	name = "Object";
}

Object::~Object()
{

}

void Object::OnUpdate(float dt)
{
}
