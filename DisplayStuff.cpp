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
#include "DisplayStuff.hpp"

int DisplayStuff::tzone;

DisplayInfo::DisplayInfo() : chkloc(0, 0), curloc(0, 0) { }

void DisplayStuff::setTime(int time) {
	std::lock_guard<duds::general::Spinlock> lock(block);
	info.now = time + toff;
}

void DisplayStuff::setTimeOffset(int o) {
	std::lock_guard<duds::general::Spinlock> lock(block);
	// adjust current time in case it is used before setTime() is called again,
	// and account for a previous offset
	info.now += o - toff;
	toff = o;
	// trigger schedule page to remake events
	info.totchg = true;
}

void DisplayStuff::setCheckLoc(const Location &l) {
	std::lock_guard<duds::general::Spinlock> lock(block);
	info.chkloc = l + poff;
	info.chkchg = info.goodfix = true;
}

void DisplayStuff::setCurrLoc(const Location &l, int le, int su) {
	std::lock_guard<duds::general::Spinlock> lock(block);
	info.curloc = l + poff;
	info.locerr = le;
	info.sats = su;
	info.poschg = info.goodfix = true;
}

void DisplayStuff::setLocOffset(const Location &o) {
	std::lock_guard<duds::general::Spinlock> lock(block);
	poff = o;
	// trigger schedule page to remake events
	info.poschg = info.totchg = true;
}

Location DisplayStuff::getLocOffset() {
	std::lock_guard<duds::general::Spinlock> lock(block);
	return poff;
}

void DisplayStuff::initBatteryData(
	const duds::data::Quantity &voltage,
	const duds::data::Quantity &power
) {
	info.battVolt = voltage.value;
	info.battPow = info.powexpmovavg = power.value;
}

void DisplayStuff::setBatteryData(
	const duds::data::Quantity &voltage,
	const duds::data::Quantity &power
) {
	std::lock_guard<duds::general::Spinlock> lock(block);
	info.battVolt = voltage.value;
	info.battPow = power.value;
	info.powexpmovavg = info.powexpmovavg * 0.9 + power.value * 0.1;
}

void DisplayStuff::initTempData(
	const duds::data::Quantity &temp,
	const duds::data::Quantity &relhum
) {
	info.temp = info.tempexpmovavg = temp.value;
	info.relhumid = info.rhumexpmovavg = relhum.value;
}

void DisplayStuff::setTempData(
	const duds::data::Quantity &temp,
	const duds::data::Quantity &relhum
) {
	std::lock_guard<duds::general::Spinlock> lock(block);
	info.temp = temp.value;
	info.tempexpmovavg = info.tempexpmovavg * 0.8 + temp.value * 0.2;
	info.relhumid = relhum.value;
	info.rhumexpmovavg = info.rhumexpmovavg * 0.8 + temp.value * 0.2;
}

void DisplayStuff::badFix() {
	std::lock_guard<duds::general::Spinlock> lock(block);
	info.poschg = true;
	info.goodfix = false;
}

void DisplayStuff::updateTotality(int s, int e, bool i) {
	std::lock_guard<duds::general::Spinlock> lock(block);
	// only record as change if the results are different
	if ((i != info.inTotality) || (s != info.start) || (e != info.end)) {
		info.inTotality = i;
		info.start = s;
		info.end = e;
		info.totchg = true;
	}
}

void DisplayStuff::setNotice(const std::string msg) {
	std::lock_guard<duds::general::Spinlock> lock(block);
	info.notetime = info.now;
	info.noticemsg = msg;
	info.notchg = true;
}

void DisplayStuff::setError(const std::string msg, int cnt) {
	std::lock_guard<duds::general::Spinlock> lock(block);
	if (info.errcnt < cnt) {
		info.errtime = info.now;
		info.errcnt = cnt;
		if (info.errormsg != msg) {
			info.errormsg = msg;
			info.errchg = true;
		}
	}
}

void DisplayStuff::clearError() {
	std::lock_guard<duds::general::Spinlock> lock(block);
	info.errcnt = 0;
	info.errormsg.clear();
}

void DisplayStuff::decError() {
	std::lock_guard<duds::general::Spinlock> lock(block);
	if (--info.errcnt <= 0) {
		info.errcnt = 0;
		//info.errormsg.clear();
	}
}

void DisplayStuff::getInfo(DisplayInfo &di) {
	std::lock_guard<duds::general::Spinlock> lock(block);
	di = info;
	info.chgflgs = 0;
}

void DisplayStuff::setTesting() {
	std::lock_guard<duds::general::Spinlock> lock(block);
	info.test = true;
}