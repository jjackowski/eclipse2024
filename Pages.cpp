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
#include "Pages.hpp"
#include "Screen.hpp"

Page::SelectionResponse ClockPage::select(
	const DisplayInfo &di,
	SelectionCause sc
) {
	if ((sc == SelectUser) || !di.goodfix || !di.errormsg.empty()) {
		return SelectPage;
	}
	return SkipPage;
}

void ClockPage::show(const DisplayInfo &di, Screen *scr) {
	// the title is used everywhere else
	scr->hideTitle();
}

void ClockPage::hide(const DisplayInfo &di, Screen *scr) { }


Page::SelectionResponse GpsPage::select(const DisplayInfo &, SelectionCause) {
	return SelectPage;
}

void GpsPage::show(const DisplayInfo &di, Screen *scr) {
	scr->showTitle("GPS Status");
}

void GpsPage::update(const DisplayInfo &di, Screen *scr) {
	std::ostringstream oss;
	if (di.goodfix) {
		scr->showText("Acc", 0, 0);
		oss << di.locerr << 'm';
		scr->showText(oss.str(), 1, 0);
		scr->showText("Sats", 2, 0);
		oss.str(std::string());
		oss << di.sats;
		scr->showText(oss.str(), 3, 0);
		oss.str(std::string());
	} else {
		scr->showText("No fix", 0, 0);
		scr->hideText(2, 0);
		scr->hideText(3, 0);
		if (di.curloc.lon == 0.0) {
			scr->hideText(1, 0);
			return;
		} else {
			scr->showText(" Old pos", 1, 0);
		}
	}
	oss << "Lon " << std::setprecision(7) << std::setw(12) << std::fixed <<
		di.curloc.lon;
	scr->showText(oss.str(), 0, 1);
	oss.str(std::string());
	oss << "Lat " << std::setprecision(7) << std::setw(12) << std::fixed <<
		di.curloc.lat;
	scr->showText(oss.str(), 0, 2);
}

void GpsPage::hide(const DisplayInfo &di, Screen *scr) {
	scr->hideText();
}


Page::SelectionResponse EclipsePage::select(
	const DisplayInfo &di,
	SelectionCause sc
) {
	if ((sc == SelectUser) || (
		// auto-select only with position fix
		di.goodfix && (
			// show if out of totality, or . . .
			!di.inTotality ||
			// . . . if the eclipse isn't over
			(di.now < (di.end + DisplayInfo::afterTotality))
		)
	)) {
		return SelectPage;
	}
	return SkipPage;
}

void EclipsePage::show(const DisplayInfo &di, Screen *scr) {
	scr->showTitle("Eclipse");
}

void EclipsePage::update(const DisplayInfo &di, Screen *scr) {
	if (di.inTotality) {
		Hms time;
		int start = di.start - DisplayInfo::beforeTotality;
		int end = di.end + DisplayInfo::afterTotality;
		if (di.now < start) {
			scr->showText("Start", 0, 0);
			time.set(start + DisplayStuff::getTimeZoneOffset());
			scr->showText(time.time(), 1, 0);
			time.set(start - di.now);
			scr->showText("Wait", 0, 1);
		} else if (di.now < end) {
			time.set(end - di.now);
			scr->showText("Remaining", 0, 1);
		} else {
			scr->showText("Over", 0, 1);
			scr->hideText(1, 1);
		}
		if (di.now < end) {
			scr->showText(time.duration(), 1, 1);
		}
		if (di.now >= start) {
			scr->showText("End", 0, 0);
			time.set(end + DisplayStuff::getTimeZoneOffset());
			scr->showText(time.time(), 1, 0);
		}
		scr->showText("Duration", 0, 2);
		time.set(end - start);
		scr->showText(time.duration(), 1, 2);
	} else {
		scr->showText("Outside totality", 0, 2);
		scr->hideText(0, 0);
		scr->hideText(1, 0);
		scr->hideText(0, 1);
		scr->hideText(1, 1);
		scr->hideText(1, 2);
	}
}

void EclipsePage::hide(const DisplayInfo &di, Screen *scr) {
	scr->hideText();
}


Page::SelectionResponse TotalityPage::select(
	const DisplayInfo &di,
	SelectionCause sc
) {
	if (di.inTotality && ((sc == SelectUser) || (di.now < (di.end)))) {
		return SelectPage;
	}
	return SkipPage;
}

void TotalityPage::show(const DisplayInfo &di, Screen *scr) {
	scr->showTitle("Totality");
}

void TotalityPage::update(const DisplayInfo &di, Screen *scr) {
	if (di.inTotality) {
		Hms time;
		if (di.now < di.start) {
			scr->showText("Start", 0, 0);
			time.set(di.start + DisplayStuff::getTimeZoneOffset());
			scr->showText(time.time(), 1, 0);
			time.set(di.start - di.now);
			scr->showText("Wait", 0, 1);
		} else if (di.now < di.end) {
			time.set(di.end - di.now);
			scr->showText("Remaining", 0, 1);
		} else {
			scr->showText("Over", 0, 1);
			scr->hideText(1, 1);
		}
		if (di.now < di.end) {
			scr->showText(time.duration(), 1, 1);
		}
		if (di.now >= di.start) {
			scr->showText("End", 0, 0);
			time.set(di.end + DisplayStuff::getTimeZoneOffset());
			scr->showText(time.time(), 1, 0);
		}
		scr->showText("Duration", 0, 2);
		time.set(di.end - di.start);
		scr->showText(time.duration(), 1, 2);
	} else {
		scr->showText("Outside totality", 0, 2);
		scr->hideText(0, 0);
		scr->hideText(1, 0);
		scr->hideText(0, 1);
		scr->hideText(1, 1);
		scr->hideText(1, 2);
	}
}

void TotalityPage::hide(const DisplayInfo &di, Screen *scr) {
	scr->hideText();
}


SystemPage::SystemPage() : lavg("/proc/loadavg") { }

Page::SelectionResponse SystemPage::select(const DisplayInfo &di, SelectionCause sc) {
	if (
		// user wants it
		(sc == SelectUser) ||
		(
			(
				di.goodfix &&
				(  // don't auto-show during the eclipse
					(di.now < (di.start - DisplayInfo::beforeTotality)) ||
					(di.now > (di.end + DisplayInfo::afterTotality))
				)
			) ||
			// show when no good GPS fix
			!di.goodfix
		)
	) {
		return SelectPage;
	}
	return SkipPage;
}

void SystemPage::show(const DisplayInfo &di, Screen *scr) {
	scr->showTitle("System");
	scr->showText("Volt", 2, 0);
	scr->showText("Pow", 2, 1);
	scr->showText("Exp", 2, 2);
	scr->showText("Load", 0, 0);
	scr->showText("avg", 0, 1);
	scr->showText(" ", 0, 2); // makes formating nice
}

void SystemPage::update(const DisplayInfo &di, Screen *scr) {
	float a[3];
	lavg >> a[0] >> a[1] >> a[2];
	lavg.seekg(0);
	std::ostringstream oss;
	for (int y = 0; y < 3; ++y) {
		oss << std::setprecision(2) << std::fixed << a[y];
		scr->showText(oss.str(), 1, y);
		oss.str(std::string());
	}
	oss << std::setprecision(1) << di.battVolt;
	scr->showText(oss.str(), 3, 0);
	oss.str(std::string());
	oss << std::setprecision(1) << di.battPow;
	scr->showText(oss.str(), 3, 1);
	oss.str(std::string());
	oss << std::setprecision(1) << di.powexpmovavg;
	scr->showText(oss.str(), 3, 2);
}

void SystemPage::hide(const DisplayInfo &di, Screen *scr) {
	scr->hideText();
}


Page::SelectionResponse SensorPage::select(
	const DisplayInfo &di,
	SelectionCause sc
) {
	if ((di.temp > 0) || (di.relhumid > 0) || (di.uv > 0)) {
		return SelectPage;
	}
	return SkipPage;
}

void SensorPage::show(const DisplayInfo &di, Screen *scr) {
	scr->showTitle("Sensors");
}

void SensorPage::update(const DisplayInfo &di, Screen *scr) {
	std::ostringstream oss;
	oss << std::fixed;
	if (di.temp > 0) {
		scr->showText("Temp", 0, 0);
		oss << std::setprecision(1) << di.temp;
		scr->showText(oss.str(), 1, 0);
		oss.str(std::string());
	}
	if (di.relhumid > 0) {
		scr->showText("Humid", 0, 1);
		oss << std::setprecision(1) << di.relhumid;
		scr->showText(oss.str(), 1, 1);
		oss.str(std::string());
	}
	if (di.uv > 0) {
		scr->showText("UV", 0, 2);
		oss << std::setprecision(1) << di.uvexpmovavg;
		scr->showText(oss.str(), 1, 2);
	}
}

void SensorPage::hide(const DisplayInfo &di, Screen *scr) {
	scr->hideText();
}
