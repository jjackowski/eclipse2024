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
#include "MenuPage.hpp"
#include "RunUi.hpp"
#include <sstream>

MenuPage::MenuPage(
	const duds::ui::graphics::BppStringCacheSptr &bsc,
	DisplayStuff &ds,
	Attention &a
) :
render(
	bsc,
	duds::ui::menu::renderers::BppMenuRenderer::ValueRightJustified |
	duds::ui::menu::renderers::BppMenuRenderer::InvertSelected
),
rootM(duds::ui::menu::MenuPage::make("Menu", 3)),
posM(duds::ui::menu::MenuPage::make("Position", 3)),
timeM(duds::ui::menu::MenuPage::make("Time", 3, 3)),
shutdownM(duds::ui::menu::MenuPage::make("Shutdown", 3, 3)),
path(rootM), psg("/", ".."), dstuff(ds), attn(a),
// the minus one on the next line fixes the frame lagging menu somehow; that
// was not the intent, but it is nice
cw(bsc->font()->estimatedMaxCharacterSize().w - 1)
{
	psg.currentHeader("[");
	psg.currentFooter("]");
	render.valueMargin(2);
	render.addScrollBar(2);
	render.toggledOnIcon(bsc->text(u8"\u00FB "));
	{
		duds::ui::menu::MenuAccess acc(shutdownM->menu());
		duds::ui::menu::GenericMenuItemSptr item =
			duds::ui::menu::GenericMenuItem::make(
				"Are you sure?",
				duds::ui::menu::MenuItem::Disabled
			);
		acc.append(std::move(item));
		item = duds::ui::menu::GenericMenuItem::make(
			"No"
		);
		//item->choseConnect([this]() { back(); });
		item->choseConnect(std::bind(&MenuPage::back, this));
		acc.append(std::move(item));
		item = duds::ui::menu::GenericMenuItem::make(
			"Yes"
		);
		//item->choseConnect([this]() { shutdown(); });
		item->choseConnect(std::bind(&MenuPage::shutdown, this));
		acc.append(std::move(item));
	}
	{
		duds::ui::menu::MenuAccess acc(posM->menu());
		duds::ui::menu::GenericMenuItemSptr itemNoOff =
			duds::ui::menu::GenericMenuItem::make(
				"No offset",
				duds::ui::menu::MenuItem::Toggle |
				duds::ui::menu::MenuItem::ToggledOn
			);
		acc.append(itemNoOff);
		// could read these from a file
		duds::ui::menu::GenericMenuItemSptr itemNebo =
			duds::ui::menu::GenericMenuItem::make(
				"Mt Nebo Park",
				duds::ui::menu::MenuItem::Toggle
			);
		acc.append(itemNebo);
		itemNoOff->choseConnect([this, itemNebo](auto &view, auto &access, auto &self) {
			dstuff.setLocOffset(Location(0, 0));
			self.setToggle();
			itemNebo->clearToggle();
		});
		itemNebo->choseConnect([this, itemNoOff](auto &view, auto &access, auto &self) {
			DisplayInfo di;
			dstuff.getInfo(di);
			dstuff.setLocOffset(Location(-93.2586701, 35.2170883) - di.curloc);
			self.setToggle();
			itemNoOff->clearToggle();
		});
	}
	{
		duds::ui::menu::MenuAccess acc(timeM->menu());
		duds::ui::menu::GenericMenuItemSptr item =
			duds::ui::menu::GenericMenuItem::make(
				"No offset",
				duds::ui::menu::MenuItem::Toggle |
				duds::ui::menu::MenuItem::ToggledOn
			);
		item->choseConnect([this](auto &view, auto &access, auto &self) {
			dstuff.setTimeOffset(0);
			attn.timeOffset(0);
			for (int i = access.size() - 1; i >= 0; --i) {
				access.clearToggle(i);
			}
			self.setToggle();
		});
		acc.append(std::move(item));
		item = duds::ui::menu::GenericMenuItem::make(
			"Eclipse start",
			duds::ui::menu::MenuItem::Toggle
		);
		item->choseConnect([this](auto &view, auto &access, auto &self) {
			DisplayInfo di;
			dstuff.getInfo(di);
			dstuff.setTimeOffset(di.start - DisplayInfo::beforeTotality - di.now);
			attn.timeOffset(di.start - DisplayInfo::beforeTotality - di.now);
			for (int i = access.size() - 1; i >= 0; --i) {
				access.clearToggle(i);
			}
			self.setToggle();
		});
		acc.append(std::move(item));
		item = duds::ui::menu::GenericMenuItem::make(
			"Totality -10 min",
			duds::ui::menu::MenuItem::Toggle
		);
		item->choseConnect([this](auto &view, auto &access, auto &self) {
			DisplayInfo di;
			dstuff.getInfo(di);
			dstuff.setTimeOffset(di.start - di.now - 600);
			attn.timeOffset(di.start - di.now - 600);
			for (int i = access.size() - 1; i >= 0; --i) {
				access.clearToggle(i);
			}
			self.setToggle();
		});
		acc.append(std::move(item));
		item = duds::ui::menu::GenericMenuItem::make(
			"Totality +10 min",
			duds::ui::menu::MenuItem::Toggle
		);
		item->choseConnect([this](auto &view, auto &access, auto &self) {
			DisplayInfo di;
			dstuff.getInfo(di);
			dstuff.setTimeOffset(di.start - di.now + 600);
			attn.timeOffset(di.start - di.now + 600);
			for (int i = access.size() - 1; i >= 0; --i) {
				access.clearToggle(i);
			}
			self.setToggle();
		});
		acc.append(std::move(item));
	}
	{
		duds::ui::menu::MenuAccess acc(rootM->menu());
		duds::ui::menu::GenericMenuItemSptr item =
			duds::ui::menu::GenericMenuItem::make(
				"Position offset"
			);
		item->choseConnect([this](auto &view, auto &access, auto &self) {
			path.push(posM);
			pageChange = true;
		});
		acc.append(std::move(item));
		item = duds::ui::menu::GenericMenuItem::make(
			"Time offset"
		);
		item->choseConnect([this](auto &view, auto &access, auto &self) {
			path.push(timeM);
			pageChange = true;
		});
		acc.append(std::move(item));
		item =
			duds::ui::menu::GenericMenuItem::make(
				"Shutdown"
			);
		/*
		item->choseConnect([this]() {
			path.push(shutdownM);
			shutdownM->view()->jump(1);
			pageChange = true;
		});
		*/
		item->choseConnect(std::bind(&MenuPage::shutdownSelect, this));
		acc.append(std::move(item));
	}
}

void MenuPage::shutdownSelect() {
	path.push(shutdownM);
	shutdownM->view()->jump(1);
	pageChange = true;
}

Page::SelectionResponse MenuPage::select(
	const DisplayInfo &di,
	SelectionCause sc
) {
	if (sc == SelectUser) {
		return SelectPage;
	}
	return SkipPage;
}

void MenuPage::show(const DisplayInfo &di, Screen *scr) {
	if (!img) {
		img = duds::ui::graphics::BppImage::make(scr->menuSize());
	}
	path.clearPastCurrent();
	psg.maxLength(scr->titleSize().w / cw + 4);
	scr->showTitle(psg.generate(path));
}

void MenuPage::hide(const DisplayInfo &di, Screen *scr) {
	scr->hideMenu();
}

void MenuPage::update(const DisplayInfo &di, Screen *scr) {
	// something to deal with frame lagging menu change
	static duds::ui::menu::MenuPage *prev;
	//duds::ui::menu::MenuPageSptr curr =
	//	std::dynamic_pointer_cast<duds::ui::menu::MenuPage>(path.currentPage());
	duds::ui::menu::MenuPage *curr =
		(duds::ui::menu::MenuPage*)(path.currentPage().get());
	assert(curr);
	curr->view()->update();
	duds::ui::menu::MenuOutputAccess moa(curr->firstMenuOutput());
	if ((curr != prev) || pageChange || moa.changed()) {
		prev = curr;
		// the rendering is a frame behind menu changes, but the title is not
		render.render(img, moa);
		scr->showTitle(psg.generate(path));
	}
	scr->showMenu(img);
	moved = pageChange = false;
}

void MenuPage::move(int offset) {
	if (offset && !moved) {
		moved = true;
		//duds::ui::menu::MenuPageSptr curr =
		//	std::dynamic_pointer_cast<duds::ui::menu::MenuPage>(path.currentPage());
		duds::ui::menu::MenuPage *curr =
			(duds::ui::menu::MenuPage*)(path.currentPage().get());
		assert(curr);
		curr->view()->backward(offset);
	}
}

void MenuPage::chose() {
	//duds::ui::menu::MenuPageSptr curr =
	//	std::dynamic_pointer_cast<duds::ui::menu::MenuPage>(path.currentPage());
	duds::ui::menu::MenuPage *curr =
		(duds::ui::menu::MenuPage*)(path.currentPage().get());
	assert(curr);
	curr->view()->chose();
}

bool MenuPage::back() {
	if (path.back()) {
		// prevents going forward on the menus, but may make navigating the
		// overall system easier and more sensible once on the menu page when
		// the system is used without rotary (EVENT_DIAL) input
		//path.clearPastCurrent();
		pageChange = true;
		return false;
	} else {
		//ru->reqPrevPage();
		return true;
	}
}

bool MenuPage::forward() {
	// never true if path.clearPastCurrent() is called in back() above
	if (path.forward()) {
		pageChange = true;
		return false;
	} else {
		//ru->reqNextPage();
		return true;
	}
}

extern std::atomic_bool quit;

void MenuPage::shutdown() {
	quit = 1;
	system("/sbin/shutdown -h now");
}
