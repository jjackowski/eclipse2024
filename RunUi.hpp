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
#include <duds/hardware/display/BppGraphicDisplay.hpp>
#include <duds/os/linux/EvdevInput.hpp>
#include <boost/signals2/shared_connection_block.hpp>
#include "Attention.hpp"
#include "Pages.hpp"
#include "Screen.hpp"

class RunUi : boost::noncopyable {
	/**
	 * Seconds to spend on a page after automatically advancing to it.
	 */
	static constexpr duds::time::interstellar::Nanoseconds pagetime =
		duds::time::interstellar::Milliseconds(11920);
	DisplayStuff &displaystuff;
	duds::hardware::display::BppGraphicDisplaySptr disp;
	duds::ui::graphics::BppImageArchiveSptr iconArc;
	/**
	 * Used to get input while waiting for it with epoll().
	 */
	duds::os::linux::Poller poller;
	// need one per device
	std::vector<duds::os::linux::EvdevInputSptr> evdevs;
	// can be used with all EvdevInput objects, but same event code will call
	// same response function
	duds::os::linux::InputHandlersSptr inhand;
	Attention attn;
	/**
	 * The clock used to sample the current time.
	 */
	duds::hardware::devices::clocks::LinuxClockSptr clock;
	/*
	 * The sample of the current time.
	 */
	//duds::hardware::devices::clocks::LinuxClock::Measurement::TimeSample ts;
	/**
	 * The time when the displayed page should automatically switch.
	 */
	duds::time::interstellar::NanoTime pageswitch;
	/**
	 * Seconds since the last midnight, UTC.
	 */
	boost::posix_time::time_duration time;
public:
	/**
	 * Page ordering.
	 */
	enum DisplayPage {
		Clock_Page,
		GPS_Status,
		Eclipse_Times,
		Totality_Times,
		Sun_Azimuth,
		Sun_Now,
		Schedule,
		System,
		Sensors,
		Network,
		Menu,
		//Error,
		//Notice,
		//InTotality,
		PageCycle         // if here, cycle to first page
	};
private:
	/**
	 * The page objects.
	 */
	std::unique_ptr<Page> pages[PageCycle];
	Screen screen;
	boost::signals2::connection movePrev, moveNext, rotor, pagePin;
	boost::signals2::connection upKey, downKey, backKey, forwardKey, selectKey;
	boost::signals2::shared_connection_block blockPrev, blockNext, blockPin;
	/**
	 * Index of the current page.
	 */
	int page;
	int testTimeOffset;
	/**
	 * Seconds remaining before automatically advancing to the next page.
	 */
	int timer;
	//bool pagechange;
	bool pagepinned = false;
	//bool syncdClock;

	void incPage(const DisplayInfo &di, Page::SelectionCause sc, int val = 1);
	void decPage(const DisplayInfo &di, Page::SelectionCause sc, int val = 1);
	void changePage(const DisplayInfo &di, Page::SelectionCause sc, int p);
	void pinPage(int val);
	void advancePageTime();

public:
	RunUi(
		duds::hardware::display::BppGraphicDisplaySptr &&gdisp,
		duds::ui::graphics::BppImageArchiveSptr &&iarc,
		const duds::hardware::devices::clocks::LinuxClockSptr &lcptr,
		DisplayStuff &dstuff,
		duds::hardware::interface::DigitalPin &buz
		//int toff
	);
	/**
	 * Initializes input innards to receive user input.
	 * @return  False if some required input events are not availble.
	 */
	bool initInput();
	/**
	 * Runs the user interface thread; will not return until application
	 * termination or unrecoverable error.
	 */
	void run();
};
