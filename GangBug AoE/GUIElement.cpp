#include "GUIElement.h"
#include "App.h"
#include "M_Render.h"

GUIElement::GUIElement(int flags) : rect(0,0,0,0)
{
	if (flags & DRAGGABLE)
		SetDraggable(true);
	else
		SetDraggable(false);

	if (flags & INTERACTIVE)
		SetInteractive(true);
	else
		SetInteractive(false);

	if (flags & CAN_FOCUS)
		SetCanFocus(true);
	else
		SetCanFocus(false);

	if (flags & ACTIVE)
		SetActive(true);
	else
		SetActive(false);
	
	if (flags & STANDARD_PRESET)
	{
		SetActive(true);
		SetCanFocus(true);
		SetInteractive(true);
		SetDraggable(false);
	}

}
GUIElement::~GUIElement()
{
	for (std::list<GUIElement*>::iterator it = childs.begin(); it != childs.end(); it++)
	{
		RELEASE(*it);
	}
	childs.clear();
}

void GUIElement::CheckInput(const GUIElement * mouseHover, const GUIElement * focus)
{
}
bool GUIElement::CheckMouseOver() const
{
	iPoint mousePosition;
	app->input->GetMousePosition(mousePosition.x, mousePosition.y);
	
	return rect.Contains(mousePosition.x, mousePosition.y);
}
void GUIElement::Center()
{
	int frame_w = (parent) ? parent->GetLocalRect().w : app->render->camera.w - (GetLocalRect().w / 2);
	int frame_h = (parent) ? parent->GetLocalRect().h : app->render->camera.h - (GetLocalRect().h / 2);

	SetLocalPos(frame_w / 2 - rect.w / 2, frame_h / 2 - rect.h / 2);
}
void GUIElement::CenterX()
{
	int frame_w = (parent) ? parent->GetLocalRect().w : app->render->camera.w;

	SetLocalPos(frame_w / 2 - rect.w / 2, rect.h);
}
void GUIElement::CenterY()
{
	int frame_h = (parent) ? parent->GetLocalRect().h : app->render->camera.h;

	SetLocalPos(rect.w, frame_h / 2 - rect.h / 2);
}
void GUIElement::AddListener(Module * moduleToAdd)
{
	bool listenerFound = false;
	for (std::list<Module*>::iterator it = listeners.begin(); it != listeners.end(); it++)
	{
		if ((*it) == moduleToAdd)
		{
			listenerFound = true;
		}
	}

	if (listenerFound) listeners.push_back(moduleToAdd);
}
void GUIElement::RemoveListener(Module * moduleToRemove) //NOTE: Check std::find
{
	std::list<Module*>::iterator it = std::find(listeners.begin(), listeners.end(), moduleToRemove);
	if (it != listeners.end())
	{
		listeners.erase(it);
	}	
}
void GUIElement::OnGuiEvent(gui_events eventToReact)
{
	std::map<gui_events, staticAnim_or_transition>::iterator it = transAndAnimations.find(eventToReact);
	if (it != transAndAnimations.end())
	{
		staticAnim_or_transition tmp = it->second;
		if (tmp < SAT_SEPARATOR)
			currentStaticAnimation = tmp;
		else if (tmp > SAT_SEPARATOR)
			currentTransition = tmp;
	}
}

GB_Rectangle<int> GUIElement::GetScreenRect() const
{
	if (parent != nullptr)
	{
		iPoint p = GetScreenPos();
		return{ p.x, p.y, rect.w, rect.h };
	}
	return rect;
}
GB_Rectangle<int> GUIElement::GetLocalRect() const
{
	return rect;
}
iPoint GUIElement::GetScreenPos() const
{
	if (parent != nullptr)
		return parent->GetScreenPos() + iPoint(rect.x, rect.y);
	else
		return iPoint(rect.x, rect.y);
}
iPoint GUIElement::GetLocalPos() const
{
	return iPoint(rect.x, rect.y);
}
iPoint GUIElement::GetSize() const
{
	return iPoint(rect.w, rect.h);
}
bool GUIElement::GetDraggable() const
{
	return status.draggable;
}
bool GUIElement::GetInteractive() const
{
	return status.interactive;
}
bool GUIElement::GetCanFocus() const
{
	return status.canFocus;
}
bool GUIElement::GetActive() const
{
	return status.active;
}
GUIElement * GUIElement::GetParent() const
{
	return parent;
}
const std::list<GUIElement*> GUIElement::GetChilds() const
{
	return childs;
}
gui_types GUIElement::GetType() const
{
	return type;
}
std::list<Module*> GUIElement::GetListeners() const
{
	return listeners;
}
bool GUIElement::GetMouseInside() const
{
	return status.mouseInside;
}
fPoint GUIElement::GetScale() const
{
	return scale;
}
bool GUIElement::GetLClicked() const
{
	return status.lClicked;
}
bool GUIElement::GetRClicked() const
{
	return status.rClicked;
}
ElementStatus GUIElement::GetElementStatus() const
{
	return status;
}

void GUIElement::SetLocalPos(int x, int y)
{
	if (parent)
	{
		rect.x = x + parent->GetLocalPos().x;
		rect.y = y + parent->GetLocalPos().y;
	}
	else
	{
		rect.x = x;
		rect.y = y;
	}
	
}
void GUIElement::SetDraggable(bool _draggable) 
{
	status.draggable = _draggable;
	status.statusChanged = true;
}
void GUIElement::SetInteractive(bool _interactive)
{
	status.interactive = _interactive;
	status.statusChanged = true;
}
void GUIElement::SetCanFocus(bool _focus)
{
	status.canFocus = _focus;
	status.statusChanged = true;
}
void GUIElement::SetActive(bool _active) 
{
	status.active = _active;
	status.statusChanged = true;
}
void GUIElement::SetParent(GUIElement * _parent) 
{
	parent = _parent;
	status.statusChanged = true;
}
void GUIElement::SetType(gui_types _type)
{
	type = _type;
	status.statusChanged = true;
}
void GUIElement::SetRectangle(GB_Rectangle<int> _rect)
{
	rect = _rect;
	status.statusChanged = true;
}
void GUIElement::SetRectangle(int x, int y, int w, int h)
{
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	status.statusChanged = true;
}
void GUIElement::SetMouseInside(bool ins)
{
	if (status.interactive && status.active)
	{
		status.mouseInside = ins;
		status.statusChanged = true;
	}

}
void GUIElement::SetScale(fPoint _scale)
{
	scale = _scale;
	resize(_scale);
	status.statusChanged = true;
}
void GUIElement::SetScale(float _scaleX, float _scaleY)
{
	scale.x = _scaleX;
	scale.y = _scaleY;
	resize(fPoint(_scaleX, _scaleY));
	status.statusChanged = true;
}
void GUIElement::SetLClicked(bool l)
{
	if (status.interactive && status.active)
	{
		status.lClicked = l;
		status.statusChanged = true;
	}
}
void GUIElement::SetRClicked(bool r)
{
	if (status.interactive && status.active)
	{
		status.rClicked = r;
		status.statusChanged = true;
	}
}
void GUIElement::SetStatusChanged(bool changed)
{
	status.statusChanged = changed;
}
void GUIElement::AddAnimationOrTransition(gui_events eventToReact, staticAnim_or_transition animOrTransition)
{
	std::map<gui_events, staticAnim_or_transition>::iterator it = transAndAnimations.find(eventToReact);
	if (it != transAndAnimations.end())
	{
		//If we find that that event has already been told to react we overide it.
		it->second = animOrTransition;
	}
	else
	{
		//If we doont find that event reaction setted already we just set it.
		transAndAnimations.insert(std::pair<gui_events, staticAnim_or_transition>(eventToReact, animOrTransition));
	}
}
void GUIElement::RemoveAnimationOrTransitionReaction(gui_events eventToReact)
{
	std::map<gui_events, staticAnim_or_transition>::iterator it = transAndAnimations.find(eventToReact);
	if (it != transAndAnimations.end())
	{
		transAndAnimations.erase(it);
	}
}
void GUIElement::GetAllAnimationAndTransitions(std::vector<std::pair<gui_events, staticAnim_or_transition>>& animsAndTrans)
{
	std::map<gui_events, staticAnim_or_transition>::iterator it = transAndAnimations.begin();
	for (; it != transAndAnimations.end(); ++it)
	{
		animsAndTrans.push_back((*it));
	}
}
bool GUIElement::HasEventReactionSet(gui_events eventToReact)
{
	std::map<gui_events, staticAnim_or_transition>::iterator it = transAndAnimations.find(eventToReact);
	return it != transAndAnimations.end();
}
staticAnim_or_transition GUIElement::GetAnimOrTransitionForEvent(gui_events eventToReact)
{
	std::map<gui_events, staticAnim_or_transition>::iterator it = transAndAnimations.find(eventToReact);
	if (it != transAndAnimations.end())
	{
		return it->second;
	}

	return SAT_NONE;
}
void GUIElement::resize(fPoint newScale)
{
	fPoint variation = fPoint(scale.x / newScale.x, scale.y / newScale.y);
	rect.x *= variation.x;
	rect.y *= variation.y;
	status.statusChanged = true;
}
void GUIElement::SetSize(int w, int h)
{
	rect.w = w;
	rect.h = h;
	status.statusChanged = true;
}

void GUIElement::Update(const GUIElement* mouseHover, const GUIElement* focus)
{
	//TODO: Add dt as a parameter. Animations really need it.

	//When updateing first do the element particular update overrided in each subtype.
	OnUpdate(mouseHover, focus);

	//Then process the animations and transitions.
	if (currentStaticAnimation != SAT_NONE)
	{
		//Do here the animation according the active one.

		//If the animation has finished set currentStaticAnimation to SAT_NONE.
	}

	if (currentTransition != SAT_NONE)
	{
		//DO here the transition logic according the one active.

		//When the transition has ended set currentTransition to SAT_NONE.
	}
}