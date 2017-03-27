#include "Unit.h"
#include "App.h"
#include "M_Animation.h"
#include "M_Render.h"

//TMP
#include "M_Input.h"
#include "App.h"

Unit::Unit(unit_type type, Entity* parent) : unitType(type), Entity(ENTITY_UNIT, parent)
{
	name.assign("unit_");
	action = IDLE;
	unitDirection = SOUTH;
	entityTexture = app->animation->GetTexture(unitType);

	iPoint p;
	app->animation->GetFrame(drawQuad, p, this);
	SetEnclosingBoxSize(drawQuad.w, drawQuad.h);
	SetPivot(p);
}

void Unit::OnUpdate(float dt)
{
	if (haveADestination == true)
	{
		fPoint gPos = GetGlobalPosition();
		fPoint desPos(destination.x, destination.y);

		if (gPos.DistanceNoSqrt(desPos) > arriveRadius * arriveRadius) //TODO: Maybe check in a circle or area instead of a point
		{
			velocity = desPos - gPos;
			velocity.Normalize();
			velocity *= (speed * dt);

			direction dir = GetDirectionFromVelocity(velocity);
			if (dir != unitDirection)
				unitDirection = dir;

			iPoint p;
			app->animation->GetFrame(drawQuad, p, this);
			SetEnclosingBoxSize(drawQuad.w, drawQuad.h);
			SetPivot(p);

			fPoint p2 = { (float)p.x, (float)p.y };
			SetGlobalPosition(gPos + velocity);
		}
		else
		{
			//Has arrived
			haveADestination = false;
			action = IDLE;
		}
	}

	
}

unit_type Unit::GetType() const
{
	return unitType;
}

action_type Unit::GetAction() const
{
	return action;
}

direction Unit::GetDirection() const
{
	return unitDirection;
}

void Unit::SetDestination(int x, int y)
{
	destination.create(x, y);
	haveADestination = true;
	action = WALK;
}

void Unit::SetDestination(iPoint dst)
{
	destination = dst;
	haveADestination = true;
	action = WALK;
}

direction Unit::GetDirectionFromVelocity(fPoint vel)
{
	direction ret = NORTH;

	float angle = vel.GetAngleDeg();

	if (angle < 22.5 && angle > -22.5)
		ret = SOUTH;//
	else if (angle >= 22.5 && angle <= 67.5)
		ret = SOUTH_EAST;
	else if (angle > 67.5 && angle < 112.5)
		ret = EAST;
	else if (angle >= 112.5 && angle <= 157.5)
		ret = NORTH_EAST;
	else if (angle > 157.5 || angle < -157.5)
		ret = NORTH;//
	else if (angle >= -157.5 && angle <= -112.5)
		ret = NORTH_WEST;
	else if (angle > -112.5 && angle < -67.5)
		ret = WEST;//
	else if (angle >= -67.5 && angle <= -22.5)
		ret = SOUTH_WEST;//

	//NOTE: SOUTH_EAST, EAST, NORTH_EAST & NORTH_WEST are missing

	return ret;
}