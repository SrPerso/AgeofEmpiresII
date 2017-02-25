#ifndef __M_GUI_H__
#define __M_GUI_H__

#include <list>
#include "Module.h"

class GUIElement;

class M_GUI : public Module
{
public:
	M_GUI(bool startEnabled = true);
	virtual ~M_GUI();

	bool Awake(pugi::xml_node&) override;
	bool PreUpdate() override;
	bool Update(float dt) override;
	bool PostUpdate() override;

	bool UpdateGuiList();
	bool UpdateDebugGuiList();

	GUIElement* FindMouseHover();
	void ManageEvents();
	void BroadcastEventToListeners(GUIElement* element, gui_events event);

	void DrawDebug() override;
	//Getters & Setters

public:
	std::list<GUIElement*> guiList;
	std::list<GUIElement*> debugGuiList;

private:
	GUIElement* mouseHover = nullptr;
	GUIElement* focus = nullptr;

};

#endif // !__M_GUI_H__