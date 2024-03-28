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
#include <duds/ui/menu/GenericMenuItem.hpp>
#include <duds/ui/menu/MenuAccess.hpp>
#include "SchedulePage.hpp"
#include "RunUi.hpp"
#include <sstream>

SchedulePage::SchedulePage(
	const duds::ui::graphics::BppStringCacheSptr &bsc,
	Attention &a
) : startT(86400), endT(86400), shownT(86400), attn(a),
menu(duds::ui::menu::Menu::make("Schedule")),
view(duds::ui::menu::MenuView::make(menu)), mout(view, 3),
render(
	bsc,
	duds::ui::menu::renderers::BppMenuRenderer::ValueRightJustified |
	duds::ui::menu::renderers::BppMenuRenderer::InvertSelected
)
{
	render.valueMargin(2);
}

Page::SelectionResponse SchedulePage::select(
	const DisplayInfo &di,
	SelectionCause sc
) {
	if (di.inTotality && (
		(sc == SelectUser) ||
		(di.now < (di.end + DisplayInfo::afterTotality + 180))
	)) {
		return SelectPage;
	}
	return SkipPage;
}

void SchedulePage::addAttn(int when) {
	attn.add(when - 60, priority, RunUi::Schedule, Attention::Notice);
	attn.add(when - 30, priority, RunUi::Schedule, Attention::Notice);
	attn.add(when, priority, RunUi::Schedule, Attention::Time);
}

void SchedulePage::makeEvents(const DisplayInfo &di) {
	evtbl.clear();
	attn.remove(RunUi::Schedule);
	if (!di.inTotality) {
		duds::ui::menu::MenuAccess acc(menu);
		acc.clear();
		return;
	}
	std::ostringstream oss;
	int midx = 0;
	evtbl.emplace(di.start - DisplayInfo::beforeTotality, Event("Start pic", ++midx));
	addAttn(di.start - DisplayInfo::beforeTotality);
	double t = double(di.start - DisplayInfo::beforeTotality);
	int cnt = 1; // pic 0 is start
	for (
		t += double(DisplayInfo::beforeTotality)/8.0;
		cnt < 8;
		++cnt, t += double(DisplayInfo::beforeTotality)/8.0
	) {
		oss << "Part pic " << cnt;
		midx += 2;
		evtbl.emplace((int)t, Event(oss.str(), midx));
		addAttn((int)t);
		oss.str(std::string());
		if (cnt == 6) {
			// I'd like to record video starting before totality.
			midx += 2;
			evtbl.emplace((int)t + 32, Event("Setup vid", midx));
			attn.add((int)t + 32, priority, RunUi::Schedule, Attention::Warning);
		}
	}
	midx += 2;
	// audible prompts for these two handled elsewhere
	evtbl.emplace(di.start, Event("Totality", midx));
	addAttn(di.start);
	midx += 2;
	evtbl.emplace(di.start + (di.end - di.start) / 2, Event("Mid-total", midx));
	attn.add(
		di.start + (di.end - di.start) / 2, priority,
		RunUi::Schedule,
		Attention::Warning
	);
	midx += 2;
	evtbl.emplace(di.end, Event("End total", midx));
	addAttn(di.end);
	for (
		t = (double)di.end + double(DisplayInfo::afterTotality)/8.0;
		cnt < 15;
		++cnt, t += double(DisplayInfo::afterTotality)/8.0
	) {
		midx += 2;
		oss << "Part pic " << cnt;
		evtbl.emplace((int)t, Event(oss.str(), midx));
		addAttn((int)t);
		oss.str(std::string());
	}
	evtbl.emplace(di.end + DisplayInfo::afterTotality, Event("End pic", midx + 2));
	addAttn(di.end + DisplayInfo::afterTotality);
	endT = di.end;
	startT = di.start;
	int startIdx = 0;
	bool future = false;
	{ // fill the menu
		duds::ui::menu::MenuAccess acc(menu);
		acc.clear();
		int cnt = 0;
		for (auto event : evtbl) {
			Hms time(event.first + DisplayStuff::getTimeZoneOffset());
			duds::ui::menu::GenericMenuItemSptr item =
				duds::ui::menu::GenericMenuItem::make(
					event.second.label,
					std::string(),
					time.time(),
					duds::ui::menu::MenuItem::Disabled
				);
			acc.append(std::move(item));
			++cnt;
			time.set(std::abs(di.now - event.first));
			// select the menu item for the event that is the least far into
			// the future
			if (!future) {
				if (di.now < event.first) {
					future = true;
				}
				startIdx = cnt;
			}
			item = duds::ui::menu::GenericMenuItem::make(
				(di.now <= event.first) ? "     In" : "   Past",
				std::string(),
				time.duration()
				//menu::MenuItem::Invisible
			);
			acc.append(std::move(item));
			++cnt;
		}
	}
	view->jump(startIdx);
}

void SchedulePage::show(const DisplayInfo &di, Screen *scr) {
	if (!img) {
		img = duds::ui::graphics::BppImage::make(scr->menuSize());
	}
	// only make the event menu when something changes
	//  the flags may be cleared before they are inspected here
	//  probably should have used boost signals to inform of the changes
	//if (di.goodfix && (di.totchg || menu->empty())) {
		makeEvents(di);
	/* } else {
		// find next event
		EventTable::const_iterator iter = evtbl.lower_bound(di.now);
		// really should be true unless now is past the last event
		if (iter != evtbl.cend()) {
			// select next event
			view->jump(iter->second.index);
		}		
	} */
	scr->showTitle("Schedule");
	render.removeScrollBar();
}

void SchedulePage::hide(const DisplayInfo &di, Screen *scr) {
	scr->hideMenu();
}

void SchedulePage::update(const DisplayInfo &di, Screen *scr) {
	// update the time list if it might have changed
	if (di.goodfix && di.totchg) {
		makeEvents(di);
	}
	{
		// update times
		duds::ui::menu::MenuAccess acc(menu);
		int cnt = 1, si = view->selectedIndex();
		for (auto event : evtbl) {
			// positive puts event in the past
			int td = di.now - event.first;
			Hms time(std::abs(td));
			duds::ui::menu::MenuItemSptr item = acc.item(cnt);
			item->value(time.duration());
			// update item label if it just changed to the past
			if ((td > 0) && (td < 2)) {
				item->label("   Past");
				// advance if selected item just changed to the past
				if (si == cnt) {
					view->backward();
				}
			}
			cnt += 2;
		}
	}
	view->update();
	render.render(img, mout);
	scr->showMenu(img);
	moved = false;
}

void SchedulePage::move(int offset) {
	if (offset && !moved) {
		moved = true;
		view->backward(offset);
		render.addScrollBar(2);
	}
}
