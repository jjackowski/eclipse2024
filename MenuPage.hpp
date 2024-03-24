/*
 * This file is part of the Eclipse2024 project. It is subject to the GPLv3
 * license terms in the LICENSE file found in the top-level directory of this
 * distribution and at
 * https://github.com/jjackowski/eclipse2024/blob/master/LICENSE.
 * No part of the Eclipse2024 project, including this file, may be copied,
 * modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 *
 * Copyright (C) 2024  Jeff Jackowski
 */
#include <duds/ui/menu/renderers/BppMenuRenderer.hpp>
#include <duds/ui/menu/MenuPage.hpp>
#include <duds/ui/PathStringGenerator.hpp>
#include "Page.hpp"
#include "Attention.hpp"
#include <map>

/**
 * Handles a menu page that acts a bit differently than other pages.
 * The menu includes these sub-menus:
 *  - Position offset: allows testing within the area of totality while using
 *    real-time GPS data from somewhere else.
 *  - Tine offset: allows testing how the system handles important events more
 *    than once a day.
 *  - Shutdown
 *
 * @author  Jeff Jackowski
 */
class MenuPage : public Page {
	duds::ui::menu::MenuPageSptr rootM, posM, timeM, shutdownM;
	duds::ui::menu::renderers::BppMenuRenderer render;
	duds::ui::graphics::BppImageSptr img;
	duds::ui::Path path;
	duds::ui::PathStringGenerator psg;
	DisplayStuff &dstuff;
	Attention &attn;
	/**
	 * Character width.
	 */
	int cw;
	bool moved = false;
	bool pageChange = true;
	void shutdownSelect();
public:
	MenuPage(
		const duds::ui::graphics::BppStringCacheSptr &bsc,
		DisplayStuff &ds,
		Attention &a
	);
	virtual Page::SelectionResponse select(
		const DisplayInfo &,
		Page::SelectionCause
	);
	virtual void show(const DisplayInfo &di, Screen *scr);
	virtual void hide(const DisplayInfo &di, Screen *scr);
	virtual void update(const DisplayInfo &di, Screen *scr);
	// positive for down
	void move(int offset);
	void moveNeg(int offset) {
		move(-offset);
	}
	void chose();
	bool back();
	bool forward();
	void shutdown();
};
