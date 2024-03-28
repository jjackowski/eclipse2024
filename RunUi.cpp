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
#include <boost/exception/diagnostic_information.hpp>
#include <duds/ui/graphics/BppFontPool.hpp>
#include "RunUi.hpp"
#include "SchedulePage.hpp"
#include "NetworkPage.hpp"
#include "MenuPage.hpp"
#include "SunPages.hpp"
#include <filesystem>

extern std::atomic_bool quit;
extern duds::ui::graphics::BppFontPool FontPool;

static const duds::os::linux::EventTypeCode EventDial = { EV_REL, REL_DIAL };
static const duds::os::linux::EventTypeCode EventSelect = { EV_KEY, BTN_SOUTH };
static const duds::os::linux::EventTypeCode EventUp = { EV_KEY, BTN_DPAD_UP };
static const duds::os::linux::EventTypeCode EventDown = { EV_KEY, BTN_DPAD_DOWN };
static const duds::os::linux::EventTypeCode EventLeft = { EV_KEY, BTN_DPAD_LEFT };
static const duds::os::linux::EventTypeCode EventRight = { EV_KEY, BTN_DPAD_RIGHT };

RunUi::RunUi(
	duds::hardware::display::BppGraphicDisplaySptr &&gdisp,
	duds::ui::graphics::BppImageArchiveSptr &&iarc,
	const duds::hardware::devices::clocks::LinuxClockSptr &lcptr,
	DisplayStuff &dstuff,
	duds::hardware::interface::DigitalPin &buz
	//int toff
) : disp(std::move(gdisp)), iconArc(std::move(iarc)), clock(lcptr), page(0),
screen(iconArc, disp->dimensions()), displaystuff(dstuff), attn(lcptr, buz)
{
	pages[Clock_Page] = std::make_unique<ClockPage>();
	pages[GPS_Status] = std::make_unique<GpsPage>();
	pages[Eclipse_Times] = std::make_unique<EclipsePage>();
	pages[Totality_Times] = std::make_unique<TotalityPage>();
	pages[Schedule] = std::make_unique<SchedulePage>(
		FontPool.getStringCache("Text"),
		attn
	);
	pages[SunAz] = std::make_unique<SunAzPage>(lcptr);
	pages[SunEl] = std::make_unique<SunElPage>(lcptr);
	pages[System] = std::make_unique<SystemPage>();
	pages[Network] = std::make_unique<NetworkPage>();
	pages[Sensors] = std::make_unique<SensorPage>();
	pages[Menu] = std::make_unique<MenuPage>(
		FontPool.getStringCache("Text"),
		dstuff,
		attn
	);
}

void RunUi::incPage(const DisplayInfo &di, Page::SelectionCause sc, int val) {
	// do nothing if a button is released
	if (val == 0) return;
	int p = page;
	Page::SelectionResponse sr = Page::SkipPage;
	do {
		if (++p >= PageCycle) {
			p = Clock_Page;
		}
		if (pages[p]) {
			sr = pages[p]->select(di, sc);
		}
	} while (sr == Page::SkipPage);
	changePage(di, sc, p);
}

void RunUi::decPage(const DisplayInfo &di, Page::SelectionCause sc, int val) {
	// do nothing if a button is released
	if (val == 0) return;
	int p = page;
	Page::SelectionResponse sr = Page::SkipPage;
	do {
		if (--p < Clock_Page) {
			p = PageCycle - 1;
		}
		if (pages[p]) {
			sr = pages[p]->select(di, sc);
		}
	} while (sr == Page::SkipPage);
	changePage(di, sc, p);
}

void RunUi::advancePageTime() {
	if (!pagepinned) {
		duds::data::Measurement::TimeSample time;
		clock->sampleTime(time);
		pageswitch = time.value + pagetime * 5;
	}
}

void RunUi::changePage(const DisplayInfo &di, Page::SelectionCause sc, int p) {
	pages[page]->hide(di, &screen);
	page = p;
	pages[page]->show(di, &screen);
	if (sc == Page::SelectUser) {
		advancePageTime();
	}
	// additional input for the schedule page
	if (page == Schedule) {
		SchedulePage *sp = (SchedulePage*)pages[Schedule].get();
		if (!sp) {
			// shouldn't happen
			return;
		}
		upKey = inhand->connect(
			EventUp,
			[sp, this](auto ig, auto val) {
				sp->moveNeg(val);
				advancePageTime();
			}
		);
		downKey = inhand->connect(
			EventDown,
			[sp, this](auto ig, auto val) {
				sp->move(val);
				advancePageTime();
			}
		);
		/*
		rotor = inhand->connect(
			EventDial,
			[sp, this](auto ig, auto val) {
				sp->move(val);
				advancePageTime();
			}
		);
		*/
	}
	// additional input for the menu page
	else if (page == Menu) {
		MenuPage *sp = (MenuPage*)pages[Menu].get();
		if (!sp) {
			// shouldn't happen
			return;
		}
		upKey = inhand->connect(
			EventUp,
			[sp, this](auto ig, auto val) {
				sp->moveNeg(val);
			}
		);
		downKey = inhand->connect(
			EventDown,
			[sp, this](auto ig, auto val) {
				sp->move(val);
			}
		);
		blockPrev.block();
		blockNext.block();
		blockPin.block();
		backKey = inhand->connect(
			EventLeft,
			[this, sp, &di](auto ig, auto val) {
				if (val) {
					if (sp->back()) {
						decPage(di, Page::SelectUser);
						advancePageTime();
					}
				}
			}
		);
		forwardKey = inhand->connect(
			EventRight,
			[this, sp, &di](auto ig, auto val) {
				if (val) {
					if (sp->forward()) {
						incPage(di, Page::SelectUser);
						advancePageTime();
					}
				}
			}
		);
		selectKey = inhand->connect(
			EventSelect,
			[sp, this](auto ig, auto val) {
				if (val) {
					sp->chose();
				}
			}
		);
		// keep page up until user or attention selects another page; avoids
		// messing with pinned page status
		pageswitch = duds::time::interstellar::NanoTime::max();
	} else {
		// if no other page is using the input, these could probably remain
		// connected with no noticeable effect
		upKey.disconnect();
		downKey.disconnect();
		//rotor.disconnect();
		backKey.disconnect();
		forwardKey.disconnect();
		selectKey.disconnect();
		blockPrev.unblock();
		blockNext.unblock();
		blockPin.unblock();
	}
}

void RunUi::pinPage(int val) {
	// do nothing if a button is released
	if (val == 0) return;
	if (pagepinned) {
		pagepinned = false;
		screen.hidePin();
		duds::data::Measurement::TimeSample time;
		clock->sampleTime(time);
		pageswitch = time.value + pagetime;
	} else {
		pagepinned = true;
		screen.showPin();
		pageswitch = duds::time::interstellar::NanoTime::max();
	}
}

struct CheckInputEvent {
	const char *name;
	duds::os::linux::EventTypeCode etc;
	bool required;
	bool found = false;
	constexpr CheckInputEvent(
		const char *n,
		duds::os::linux::EventTypeCode c,
		bool req = true
	) : name(n), etc(c), required(req) { }
};

bool RunUi::initInput() {
	CheckInputEvent checkInputEvents[] = {
		{ "dial", EventDial, false },
		{ "select", EventSelect },
		{ "up", EventUp },
		{ "down", EventDown },
		{ "left", EventLeft },
		{ "right", EventRight }
	};
	int found = 0;
	const std::filesystem::path inputdevpath("/dev/input");
	for (auto const &infile : std::filesystem::directory_iterator(inputdevpath)) {
		if (!infile.is_character_file()) continue;
		std::cout << "Trying " << infile.path() << std::endl;
		duds::os::linux::EvdevInputSptr eis;
		try {
			eis = duds::os::linux::EvdevInput::make(infile.path());
		} catch (...) { }
		if (!eis) continue;
		// look for input events
		bool keep = false;
		for (CheckInputEvent &cie : checkInputEvents) {
			if (eis->hasEvent(cie.etc)) {
				if (!cie.found) {
					cie.found = true;
					++found;
				}
				keep = true;
			}
		}
		if (keep) {
			std::cout << "\tkeeping" << std::endl;
			evdevs.emplace_back(std::move(eis));
		}
	}
	// show inputs found
	for (CheckInputEvent &cie : checkInputEvents) {
		if (cie.required) {
			if (cie.found) {
				std::cout << "Found required input for ";
			} else {
				std::cout << "MISSING REQUIRED input for ";
			}
		} else {
			if (cie.found) {
				std::cout << "Found optional input for ";
			} else {
				std::cout << "Missing optional input for ";
			}
		}
		std::cout << cie.name << std::endl;
	}
	// make input handlers; used even if no input
	inhand = std::make_shared<duds::os::linux::InputHandlers>();
	// check for adequate input
	const auto numEvents = std::end(checkInputEvents) - std::begin(checkInputEvents);
	if (
		(found == numEvents) ||
		((found == (numEvents - 1)) && !checkInputEvents[0].found)
	) {
		// provide input handlers
		for (const duds::os::linux::EvdevInputSptr &eis : evdevs) {
			eis->connect(inhand);
			eis->usePoller(poller);
		}
		// eat input
		for (int limit = 64; limit && (poller.respond() == poller.maxEvents); --limit) { }
		return true;
	}
	return false;
}

void RunUi::run()
try {
	duds::ui::graphics::BppImage frame(disp->dimensions());
	DisplayInfo dinfo;
	duds::data::Measurement::TimeSample time;
	duds::data::Measurement::TimeSample::Value stime;
	clock->sampleTime(time);
	stime = time.value;
	pageswitch = time.value + pagetime;
	movePrev = inhand->connect(
		EventLeft,
		std::bind(
			&RunUi::decPage,
			this,
			std::ref(dinfo),
			Page::SelectUser,
			std::placeholders::_2
		)
	);
	blockPrev = boost::signals2::shared_connection_block(movePrev, false);
	moveNext = inhand->connect(
		EventRight,
		std::bind(
			&RunUi::incPage,
			this,
			std::ref(dinfo),
			Page::SelectUser,
			std::placeholders::_2
		)
	);
	blockNext = boost::signals2::shared_connection_block(moveNext, false);
	rotor = inhand->connect(
		EventDial,
		[this, &dinfo](auto ig, auto val) {
			if (val > 0) {
				incPage(dinfo, Page::SelectUser, val);
			} else {
				decPage(dinfo, Page::SelectUser, val);
			}
		}
	);
	//blockRotor = boost::signals2::shared_connection_block(rotor, false);
	pagePin = inhand->connect(
		EventSelect,
		std::bind(
			&RunUi::pinPage,
			this,
			std::placeholders::_2
		)
	);
	blockPin = boost::signals2::shared_connection_block(pagePin, false);
	// used to prevent page changes during critical times
	bool pagechange = true;
	// UI loop
	do {
		clock->sampleTime(time);
		// advance time by 64ms (the display is slow)
		time.value += duds::time::interstellar::Milliseconds(64);
		displaystuff.setTime(
			duds::time::planetary::earth->posix(
				time.value
			).time_of_day().total_seconds()
		);
		displaystuff.getInfo(dinfo);
		if (dinfo.errormsg.empty()) {
			screen.hideInfoMsg();
		} else {
			screen.showInfoMsg(dinfo.errormsg);
		}
		if (pagechange) {
			// change on attention event
			int chgp = attn.changeToPage();
			if (chgp > 0) {
				changePage(dinfo, Page::SelectUser, chgp);
				if (!pagepinned) {
					pageswitch = time.value + pagetime * 5;
				}
			} else {
				// auto page change
				if (time.value > pageswitch) {
					incPage(dinfo, Page::SelectAuto);
					pageswitch = time.value + pagetime;
				}
			}
		}
		// update the displayed page
		pages[page]->update(dinfo, &screen);
		// show the time 64ms in the future; it'll take a while to update the
		// display
		screen.render(frame, time);
		disp->write(&frame);
		// wait on input until 60ms before the next second
		if (
			poller.wait(
				std::max(
					// delay to next second
					duds::time::interstellar::Milliseconds(1000) -
					(
						duds::time::interstellar::MilliClock::now().time_since_epoch() %
						duds::time::interstellar::Milliseconds(1000)
					)
					- duds::time::interstellar::Milliseconds(60),
					// minimum delay; ensure not <= 0
					duds::time::interstellar::Milliseconds(1)
				)
			) == poller.maxEvents
		) {
			// get more input
			for (int limit = 8; limit && (poller.respond() == poller.maxEvents); --limit) { }
		}
	} while (!quit);
} catch (...) {
	std::cerr << "Program failed in RunUi::run():\n" <<
	boost::current_exception_diagnostic_information() << std::endl;
	quit = true;
}
