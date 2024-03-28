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
#include "SunPages.hpp"
#include "Screen.hpp"

Page::SelectionResponse SunAzPage::select(
	const DisplayInfo &di,
	SelectionCause sc
) {
	if (di.goodfix && (
		(sc == SelectUser) ||
		// don't auto-show during the eclipse
		(di.now < (di.start - DisplayInfo::beforeTotality)) ||
		(di.now > (di.end + DisplayInfo::afterTotality))
	)) {
		return SelectPage;
	}
	return SkipPage;
}

void SunAzPage::show(const DisplayInfo &di, Screen *scr) {
	scr->showTitle("Sun Azimuth");
	scr->showText("Current", 0, 0);
	scr->showText("Mid-total", 0, 1);
	scr->showText("Diff", 0, 2);
	scr->showText("Rng", 2, 2);
	rcnt = 0;
}

void SunAzPage::update(const DisplayInfo &di, Screen *scr) {
	// limit recomputations
	if (--rcnt < 0) {
		rcnt = 8;
		duds::data::Measurement::TimeSample ts;
		double naz, nel, taz, tel, saz, eaz, el;
		int midt = (di.end - di.start) / 2 + di.start;
		clock->sampleTime(ts);
		// current position
		sunPosition(naz, nel, di.curloc, ts.value);
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(0) << naz;
		scr->showText(oss.str(), 1, 0);
		// inside totality?
		if (di.inTotality) {
			// position at mid-totality
			sunPosition(
				taz,
				tel,
				di.curloc,
				ts.value + duds::time::interstellar::Seconds(midt - di.now)
			);
			// position at eclipse start
			sunPosition(
				saz,
				el,
				di.curloc,
				ts.value + duds::time::interstellar::Seconds(
					di.start - DisplayInfo::beforeTotality - di.now
				)
			);
			// position at eclipse end
			sunPosition(
				eaz,
				el,
				di.curloc,
				ts.value + duds::time::interstellar::Seconds(
					di.end + DisplayInfo::afterTotality - di.now
				)
			);
			oss.str(std::string());
			oss << taz;
			scr->showText(oss.str(), 1, 1);
			oss.str(std::string());
			oss << (taz - naz);
			scr->showText(oss.str(), 1, 2);
			oss.str(std::string());
			oss << std::fabs(eaz - saz);
			scr->showText(oss.str(), 3, 2);
		} else {
			scr->showText("N/A", 1, 1);
			scr->showText("N/A", 1, 2);
			scr->showText("N/A", 3, 2);
		}
	}
}

void SunAzPage::hide(const DisplayInfo &di, Screen *scr) {
	scr->hideText();
}


Page::SelectionResponse SunElPage::select(
	const DisplayInfo &di,
	SelectionCause sc
) {
	if (di.goodfix && (
		(sc == SelectUser) ||
		// don't auto-show during the eclipse
		(di.now < (di.start - DisplayInfo::beforeTotality)) ||
		(di.now > (di.end + DisplayInfo::afterTotality))
	)) {
		return SelectPage;
	}
	return SkipPage;
}

void SunElPage::show(const DisplayInfo &di, Screen *scr) {
	scr->showTitle("Sun Elevation");
	scr->showText("Current", 0, 0);
	scr->showText("Mid-total", 0, 1);
	scr->showText("Peak", 0, 2);
	rcnt = 0;
}

void SunElPage::update(const DisplayInfo &di, Screen *scr) {
	// limit recomputations
	if (--rcnt < 0) {
		rcnt = 8;
		duds::data::Measurement::TimeSample ts;
		double az, nel, el;
		clock->sampleTime(ts);
		// current position
		sunPosition(az, nel, di.curloc, ts.value);
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(0) << nel;
		scr->showText(oss.str(), 1, 0);
		if (di.inTotality) {
			int newmidt = (di.end - di.start) / 2 + di.start;
			if ((newmidt != midt) || di.totchg || di.poschg) {
				midt = newmidt;
				// position at mid-totality
				sunPosition(
					az,
					tel,
					di.curloc,
					ts.value + duds::time::interstellar::Seconds(midt - di.now)
				);
				// find peak
				duds::time::interstellar::SecondTime chkt =
					ts.value + duds::time::interstellar::Seconds(
						di.start - DisplayInfo::beforeTotality - di.now
					);
				duds::time::interstellar::SecondTime endt =
					ts.value + duds::time::interstellar::Seconds(
						di.end + DisplayInfo::afterTotality - di.now
					);
				peak = 0;
				do {
					sunPosition(az, el, di.curloc, chkt);
					peak = std::max(peak, el);
					chkt += duds::time::interstellar::Seconds(64);
				} while (chkt < endt);
			}
			oss.str(std::string());
			oss << tel;
			scr->showText(oss.str(), 1, 1);
			oss.str(std::string());
			oss << peak;
			scr->showText(oss.str(), 1, 2);
		} else {
			scr->showText("N/A", 1, 1);
			scr->showText("N/A", 1, 2);
		}
	}
}

void SunElPage::hide(const DisplayInfo &di, Screen *scr) {
	scr->hideText();
}
