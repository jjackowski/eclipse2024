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
#include "TimePanel.hpp"
#include <duds/time/planetary/Planetary.hpp>
#include <duds/ui/graphics/BppFontPool.hpp>
#include <duds/hardware/devices/clocks/LinuxClock.hpp>

extern duds::ui::graphics::BppFontPool FontPool;

// ordered by size-step
static const std::string fontNames[] = {
	"Time-large",
	"Time-medium",
	"Time-medium",
	"Time-small",
	"Time-tiny",
	"Time-tiny",
};

TimePanel::TimePanel(Token) : nextSec(duds::time::interstellar::SecondTime::min()) { }

void TimePanel::updateTime(const duds::data::Measurement::TimeSample &ts) {
	// using timestamp here is a shortcut, but should work fine
	if (duds::time::interstellar::SecondTime(ts.value) >= nextSec) {
		// compute and store the time of the next second
		//nextSec += duds::time::interstellar::Seconds(1);
		nextSec = duds::time::interstellar::SecondTime(ts.value) +
			duds::time::interstellar::Seconds(1);
		// get the new day in computer's set time zone
		std::time_t tt = duds::time::planetary::earth->timeUtc(
			ts.value
		);
		std::tm ltime;
		localtime_r(&tt, &ltime);
		// clear rendered time images
		for (int i = 0; i < 4; ++i) {
			outputs[i].reset();
		}
		if (ltime.tm_min != time.tm_min) {
			outputs[4].reset();
		}
		time = ltime;
	}
}

const duds::ui::graphics::BppImage *TimePanel::render(
	duds::ui::graphics::ImageLocation &offset,
	duds::ui::graphics::ImageDimensions &dim,
	duds::ui::graphics::PanelMargins &margin,
	int sizeStep
) {
	assert(sizeStep < 5);
	// does it need to be rendered?
	if (!outputs[sizeStep]) {
		std::ostringstream oss;
		// don't show seconds?
		if ((sizeStep == 2) || (sizeStep > 4)) {
			// time not set?
			if (time.tm_year < 100) {
				oss << "??:??";
			} else {
				oss << std::setfill('0') << std::setw(1) << time.tm_hour << ':'
				<< std::setw(2) << time.tm_min;
			}
		} else {
			char sep;
			if (blink && (time.tm_sec & 1)) {
				sep = ' ';
			} else {
				sep = ':';
			}
			// time not set?
			if (time.tm_year < 100) {
				oss << "??" << sep << "??" << sep << "??";
			} else {
				oss << std::setfill('0') << std::setw(1) << time.tm_hour << sep
				<< std::setw(2) << time.tm_min << sep << std::setw(2) <<
				time.tm_sec;
			}
		}
		outputs[sizeStep] = FontPool.render(fontNames[sizeStep], oss.str());
	}
	dim = outputs[sizeStep]->dimensions();
	if (sizeStep >= 3) {
		// the last column will be empty; don't show it
		--dim.w;
	}
	return outputs[sizeStep].get();
}
