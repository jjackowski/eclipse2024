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
#ifndef DISPLAYSTUFF_HPP
#define DISPLAYSTUFF_HPP

#include <duds/general/Spinlock.hpp>
#include <duds/data/Quantity.hpp>
#include "Functions.hpp"

struct DisplayInfo {
	Location chkloc;  // for checked position
	Location curloc;
	std::string errormsg;
	std::string noticemsg;
	double battVolt = 0;
	double battPow = 0;
	double powexpmovavg = 0; // power exponential moving average
	double temp = 0;
	double tempexpmovavg = 0;
	double relhumid = 0;
	double rhumexpmovavg = 0;
	double uv = 0;
	double uvexpmovavg = 0;
	int now;    // seconds since midnight UTC
	int errtime;
	int notetime;
	int errcnt; // number of times error will be shown
	int locerr;
	int start = 86400;
	int end = 86400;
	int sats;
	union {
		std::uint8_t chgflgs = 0;
		struct {
			int totchg : 1; // totality change; start, end, and inTotality changed
			int poschg : 1;
			int chkchg : 1;
			int errchg : 1;
			int notchg : 1;
		};
	};
	bool inTotality = false;
	bool goodfix = false;
	bool test = false;
	DisplayInfo();
	/**
	 * Kludge for figuring when the eclipse starts; should be good for
	 * Mount Nebo State Park, Arkansas. Puts the start 1h17m02s before totality.
	 */
	static constexpr int beforeTotality = 4622;
	/**
	 * Kludge for figuring when the eclipse ends; should be good for
	 * Mount Nebo State Park, Arkansas. Puts the end 1h16m29s after totality.
	 */
	static constexpr int afterTotality = 4589;
};

class DisplayStuff {
	DisplayInfo info;
	Location poff = Location(0, 0);
	duds::general::Spinlock block;
	int toff = 0;
	static int tzone;
public:
	friend class DatePanel;
	static int getTimeZoneOffset() {
		return tzone;
	}
	void setTime(int time);
	void setTimeOffset(int o);
	void setCheckLoc(const Location &l);
	void setCurrLoc(const Location &l, int le, int su);
	void setLocOffset(const Location &o);
	Location getLocOffset();
	void initBatteryData(
		const duds::data::Quantity &voltage,
		const duds::data::Quantity &power
	);
	void setBatteryData(
		const duds::data::Quantity &voltage,
		const duds::data::Quantity &power
	);
	void initTempData(
		const duds::data::Quantity &temp,
		const duds::data::Quantity &relhum
	);
	void setTempData(
		const duds::data::Quantity &temp,
		const duds::data::Quantity &relhum
	);
	void badFix();
	void updateTotality(int s, int e, bool i);
	void setError(const std::string msg, int cnt);
	void setNotice(const std::string msg);
	void clearError();
	void decError();
	void getInfo(DisplayInfo &di);
	bool wasGood() const {
		return info.goodfix;  // only used by one thread
	}
	void setTesting();
	bool isTesting() const {
		return info.test;  // won't change after init
	}
};

#endif        //  #ifndef DISPLAYSTUFF_HPP
