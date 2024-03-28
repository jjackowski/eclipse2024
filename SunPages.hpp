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
#include <duds/hardware/devices/clocks/LinuxClock.hpp>
#include "Page.hpp"

/**
 * Sun azimuth page.
 */
class SunAzPage : public Page {
	duds::hardware::devices::clocks::LinuxClockSptr clock;
	int rcnt;
public:
	SunAzPage(const duds::hardware::devices::clocks::LinuxClockSptr &clk) :
		clock(clk) { }
	virtual SelectionResponse select(
		const DisplayInfo &di,
		SelectionCause cause
	);
	virtual void show(const DisplayInfo &di, Screen *scr);
	virtual void hide(const DisplayInfo &di, Screen *scr);
	virtual void update(const DisplayInfo &di, Screen *scr);
};

/**
 * Sun elevation page.
 */
class SunElPage : public Page {
	duds::hardware::devices::clocks::LinuxClockSptr clock;
	double tel;
	double peak;
	int midt;
	int rcnt;
public:
	SunElPage(const duds::hardware::devices::clocks::LinuxClockSptr &clk) :
		clock(clk) { }
	virtual SelectionResponse select(
		const DisplayInfo &di,
		SelectionCause cause
	);
	virtual void show(const DisplayInfo &di, Screen *scr);
	virtual void hide(const DisplayInfo &di, Screen *scr);
	virtual void update(const DisplayInfo &di, Screen *scr);
};
