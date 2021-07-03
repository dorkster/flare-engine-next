/*
Copyright © 2011-2012 Clint Bellanger
Copyright © 2012-2015 Justin Jacobs
Copyright © 2013 Kurt Rinnert
Copyright © 2014 Henrik Andersson

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
 * class WidgetScrollBar
 */

#include "EngineSettings.h"
#include "InputState.h"
#include "RenderDevice.h"
#include "SharedResources.h"
#include "WidgetScrollBar.h"

const std::string WidgetScrollBar::DEFAULT_FILE = "images/menus/buttons/scrollbar_default.png";

WidgetScrollBar::WidgetScrollBar(const std::string& _fileName)
	: Widget()
	, fileName(_fileName)
	, scrollbars(NULL)
	, value(0)
	, bar_height(0)
	, maximum(0)
	, lock_main1(false)
	, dragging(false)
	, bg(NULL)
	, pressed_up(false)
	, pressed_down(false)
	, pressed_knob(false) {

	loadArt();

	if (scrollbars) {
		pos_up.w = pos_down.w  = pos_knob.w = scrollbars->getGraphicsWidth();
		pos_up.h = pos_down.h = pos_knob.h = (scrollbars->getGraphicsHeight() / 5); //height of one button
	}
}

void WidgetScrollBar::loadArt() {
	Image *graphics = NULL;
	if (fileName != DEFAULT_FILE) {
		graphics = render_device->loadImage(fileName, RenderDevice::ERROR_NORMAL);
	}
	if (!graphics) {
		graphics = render_device->loadImage(DEFAULT_FILE, RenderDevice::ERROR_EXIT);
	}
	if (graphics) {
		scrollbars = graphics->createSprite();
		graphics->unref();
	}
}

int WidgetScrollBar::checkClick() {
	return checkClickAt(inpt->mouse.x,inpt->mouse.y);
}

/**
 * Sets and releases the "pressed" visual state of the ScrollBar
 * If press and release, activate (return 1 for up, 2 for down)
 */
int WidgetScrollBar::checkClickAt(int x, int y) {
	Point mouse = Point(x,y);

	bool in_bounds = Utils::isWithinRect(getBounds(), mouse);
	bool in_up = Utils::isWithinRect(pos_up, mouse) || Utils::isWithinRect(up_to_knob, mouse);
	bool in_down = Utils::isWithinRect(pos_down, mouse) || Utils::isWithinRect(knob_to_down, mouse);
	bool in_knob = Utils::isWithinRect(pos_knob, mouse);

	// detect new click
	if (in_bounds && (!lock_main1 || dragging)) {
		lock_main1 = false;
		dragging = false;

		if (inpt->pressing[Input::MAIN1]) {
			inpt->lock[Input::MAIN1] = true;
			if (in_up && !pressed_knob) {
				pressed_up = true;
			}
			else if (in_down && !pressed_knob) {
				pressed_down = true;
			}
			else if (in_knob && !pressed_up && !pressed_down) {
				pressed_knob = true;
				dragging = true;
			}
			else if (in_bounds && pressed_knob) {
				dragging = true;
			}
		}
	}
	else {
		lock_main1 = inpt->pressing[Input::MAIN1];
	}

	int ret = 0;
	// main click released, so the ScrollBar state goes back to unpressed
	if (pressed_up && !inpt->pressing[Input::MAIN1]) {
		pressed_up = false;
		if (in_up) {
			// activate upon release
			ret = 1;
		}
	}
	else if (pressed_down && !inpt->pressing[Input::MAIN1]) {
		pressed_down = false;
		if (in_down) {
			// activate upon release
			ret = 2;
		}
	}
	else if (pressed_knob && dragging) {
		int tmp = mouse.y - pos_up.y - pos_up.h;

		if (bar_height < 1) bar_height = 1;
		value = (tmp * maximum)/bar_height;
		set();

		ret = 3;
	}

	if (!inpt->pressing[Input::MAIN1]) {
		dragging = false;
		pressed_knob = false;
		pressed_up = false;
		pressed_down = false;
	}

	return ret;

}

void WidgetScrollBar::set() {
	if (maximum < 1) maximum = 1;
	value = std::max(0, std::min(maximum, value));
	pos_knob.y = pos_up.y + pos_up.h + (value * (bar_height - pos_up.h) / maximum);

	up_to_knob.x = knob_to_down.x = pos_knob.x;
	up_to_knob.w = knob_to_down.w = pos_knob.w;
	up_to_knob.y = pos_up.y + pos_up.h;
	up_to_knob.h = pos_knob.y - up_to_knob.y;
	knob_to_down.y = pos_knob.y + pos_knob.h;
	knob_to_down.h = pos_down.y - knob_to_down.y;
}

int WidgetScrollBar::getValue() {
	return value;
}

Rect WidgetScrollBar::getBounds() {
	Rect r;
	r.x = pos_up.x;
	r.y = pos_up.y;
	r.w = pos_up.w;
	r.h = (pos_up.h * 2) + bar_height;

	return r;
}

void WidgetScrollBar::render() {
	Rect src_up, src_down, src_knob;

	src_up.x = 0;
	src_up.y = (pressed_up ? pos_up.h : 0);
	src_up.w = pos_up.w;
	src_up.h = pos_up.h;

	src_down.x = 0;
	src_down.y = (pressed_down ? pos_down.h*3 : pos_down.h*2);
	src_down.w = pos_down.w;
	src_down.h = pos_down.h;

	src_knob.x = 0;
	src_knob.y = pos_knob.h * 4;
	src_knob.w = pos_knob.w;
	src_knob.h = pos_knob.h;

	if (bg) {
		bg->local_frame = local_frame;
		bg->setOffset(local_offset);
		bg->setDestFromRect(pos_up);
		render_device->render(bg);
	}
	if (scrollbars) {
		scrollbars->local_frame = local_frame;
		scrollbars->setOffset(local_offset);

		scrollbars->setClipFromRect(src_up);
		scrollbars->setDestFromRect(pos_up);
		render_device->render(scrollbars);

		scrollbars->setClipFromRect(src_down);
		scrollbars->setDestFromRect(pos_down);
		render_device->render(scrollbars);

		scrollbars->setClipFromRect(src_knob);
		scrollbars->setDestFromRect(pos_knob);
		render_device->render(scrollbars);
	}
}

/**
 * Updates the scrollbar's location
 */
void WidgetScrollBar::refresh(int x, int y, int h, int val, int max) {
	Rect before = getBounds();
	maximum = max;
	value = val;
	pos_up.x = pos_down.x = pos_knob.x = x;
	pos_up.y = y;
	pos_down.y = y+h;
	bar_height = pos_down.y-(pos_up.y+pos_up.h);
	set();

	Rect after = getBounds();
	if (before.h != after.h) {
		// create background surface
		if (bg) {
			delete bg;
			bg = NULL;
		}
		Image *graphics;
		graphics = render_device->createImage(after.w,after.h);
		if (graphics) {
			bg = graphics->createSprite();
			graphics->unref();
		}

		if (bg) {
			bg->getGraphics()->fillWithColor(eset->widgets.scrollbar_bg_color);
		}
	}
}

WidgetScrollBar::~WidgetScrollBar() {
	if (scrollbars) delete scrollbars;
	if (bg) delete bg;
}

