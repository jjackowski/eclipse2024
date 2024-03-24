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
#include "Page.hpp"
#include "Attention.hpp"
#include <map>

class SchedulePage : public Page {
	struct Event {
		std::string label;
		int index;
		Event() = default;
		Event(const std::string &lbl, int idx) : label(lbl), index(idx) { }
	};
	typedef std::map<int, Event> EventTable;
	// time key in UTC with 0 being midnight; no date
	EventTable evtbl;
	Attention &attn;
	duds::ui::menu::MenuSptr menu;
	duds::ui::menu::MenuViewSptr view;
	duds::ui::menu::MenuOutput mout;
	duds::ui::menu::renderers::BppMenuRenderer render;
	duds::ui::graphics::BppImageSptr img;
	int startT, endT, shownT;
	// used to prevent handling input too many times
	bool moved = false;
	void addAttn(int when);
	void makeEvents(const DisplayInfo &);
	static constexpr int priority = 3;
public:
	SchedulePage(const duds::ui::graphics::BppStringCacheSptr &bsc, Attention &a);
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
};
