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
#include "DatePanel.hpp"
#include <duds/time/planetary/Planetary.hpp>
#include <duds/ui/graphics/BppFontPool.hpp>
#include <duds/hardware/devices/clocks/LinuxClock.hpp>
#include "DisplayStuff.hpp"

extern duds::ui::graphics::BppFontPool FontPool;

// ordered by size-step
static const std::string fontNames[] = {
	//"Date-medium",
	//"Date-small",
	"Date-medium",
	"Date-small",
};

DatePanel::DatePanel(Token) : nextDay(duds::time::interstellar::SecondTime::min()) {
	// configure a date facet for making the date string
	boost::gregorian::date_facet *dateform =
		new boost::gregorian::date_facet("%a %b %e, %Y");
	oss.imbue(std::locale(oss.getloc(), dateform));
}

void DatePanel::updateTime(const duds::data::Measurement::TimeSample &ts) {
	// using timestamp here is a shortcut, but should work fine
	if (ts.value >= nextDay) {
		// get the new day in computer's set time zone
		std::time_t tt = duds::time::planetary::earth->timeUtc(
			ts.value
		);
		std::tm ltime;
		localtime_r(&tt, &ltime);
		date = boost::gregorian::date_from_tm(ltime);
		// record time zone
		DisplayStuff::tzone = ltime.tm_gmtoff;
		// compute and store the time of the next day
		++ltime.tm_mday;
		ltime.tm_sec = ltime.tm_min = ltime.tm_hour = 0;
		tt = mktime(&ltime);
		// time zone may have changed
		std::tm ntime;
		localtime_r(&tt, &ntime);
		duds::time::planetary::earth->time(
			nextDay,
			boost::posix_time::from_time_t(tt) +
			// for time zone change
			boost::posix_time::seconds(ltime.tm_gmtoff - ntime.tm_gmtoff)
		);
		// clear rendered date images
		for (duds::ui::graphics::BppImageSptr &out : outputs) {
			out.reset();
		}
		// clear date strings
		dateStr.clear();
	}
}

const duds::ui::graphics::BppImage *DatePanel::render(
	duds::ui::graphics::ImageLocation &offset,
	duds::ui::graphics::ImageDimensions &dim,
	duds::ui::graphics::PanelMargins &margin,
	int sizeStep
) {
	// find widest date that fits
	for (int i = 0; i < 2; ++i) {
		if (!outputs[i]) {
			if (dateStr.empty()) {
				// produce date string
				oss << date << std::ends;
				dateStr = oss.str();
				if (dateStr.back() == 0) {
					dateStr.pop_back();
				}
				oss.clear();
				oss.seekp(0);
			}
			// render image
			duds::ui::graphics::BppFontSptr font = FontPool.getFont(fontNames[i]);
			if (!font) continue; // font really should be there
			// render the date
			outputs[i] = font->render(dateStr);
			// length without year
			duds::ui::graphics::ImageDimensions dim = font->lineDimensions(
				dateStr.substr(0, dateStr.size() - 6)
			);
			widths[i] = dim.w;
		}
		// width check with year
		if (outputs[i]->dimensions().w <= dim.w) {
			dim = outputs[sizeStep]->dimensions();
			return outputs[i].get();
		}
		// width check without year
		if (widths[i] <= dim.w) {
			dim = outputs[sizeStep]->dimensions();
			dim.w = widths[i];
			return outputs[i].get();
		}
	}
	// if execution gets here, panel layout is misconfigured
	return nullptr;
}
