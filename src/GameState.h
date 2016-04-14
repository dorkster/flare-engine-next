/*
Copyright © 2011-2012 kitano
Copyright © 2014-2015 Justin Jacobs

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


#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "CommonIncludes.h"

class GameState {
public:
	GameState();
	virtual ~GameState();

	virtual void logic();
	virtual void render();
	virtual void refreshWidgets();

	GameState* getRequestedGameState();
	bool isExitRequested() {
		return exitRequested;
	}
	void setLoadingFrame();
	virtual bool isPaused();

	bool hasMusic;
	bool has_background;
	bool reload_music;
	bool reload_backgrounds;
	bool save_settings_on_exit;

	int load_counter;

protected:
	GameState* requestedGameState;
	bool exitRequested;
};

#endif
