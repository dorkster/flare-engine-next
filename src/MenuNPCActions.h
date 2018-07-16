/*
Copyright © 2013-2014 Henrik Andersson
Copyright © 2013-2014 Justin Jacobs

This file is part of FLARE.

FLARE is free software: you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

FLARE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
FLARE.  If not, see http://www.gnu.org/licenses/
*/

/**
 * class MenuNPCActions
 */

#ifndef MENU_NPC_ACTIONS_H
#define MENU_NPC_ACTIONS_H

#include "CommonIncludes.h"
#include "Utils.h"

class NPC;
class WidgetLabel;

class MenuNPCActions : public Menu {
private:
	class Action {
	public:
		Action(const std::string& _id = "", const std::string& _label = "");
		~Action();
		Action(const Action &r);
		Action& operator=(const Action &r);

		std::string id;
		WidgetLabel *label;
		Rect rect;
	};

	static const int SEPARATOR_HEIGHT = 2;
	static const int ITEM_SPACING = 2;
	static const int MENU_BORDER = 8;

	typedef std::vector<Action> ActionsContainer;
	typedef ActionsContainer::iterator ActionsIterator;

	ActionsContainer npc_actions;
	NPC *npc;

	bool is_selected;
	bool is_empty;
	int first_dialog_node;
	size_t current_action;

	Sprite *action_menu;
	Color topic_normal_color;
	Color topic_hilight_color;
	Color vendor_normal_color;
	Color vendor_hilight_color;
	Color cancel_normal_color;
	Color cancel_hilight_color;

	std::string vendor_label;
	std::string cancel_label;

	Color background_color;
	Point base_pos;

	void keyboardLogic();

public:
	MenuNPCActions();
	~MenuNPCActions();

	void setNPC(NPC *npc);

	bool empty();
	void logic();
	void render();
	void update();

	bool selection();

	bool dialog_selected;
	bool vendor_selected;
	bool cancel_selected;
	int selected_dialog_node;

};


#endif
