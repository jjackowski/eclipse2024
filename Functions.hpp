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
#include <duds/time/interstellar/Interstellar.hpp>
#include <iostream>

struct Location {
	double lon;
	double lat;
	Location() = default;
	constexpr Location(double x, double y) : lon(x), lat(y) { }
	/**
	 * Adds two Location objects together; useful for testing with a position
	 * offset, but results in distance distortions.
	 */
	Location operator + (const Location &l) const {
		return Location(lon + l.lon, lat + l.lat);
	}
	/**
	 * Adds two Location objects together; useful for testing with a position
	 * offset, but results in distance distortions.
	 */
	Location &operator += (const Location &l)  {
		lon += l.lon;
		lat += l.lat;
		return *this;
	}
	/**
	 * Subtracts two Location objects together; useful for testing with a
	 * position offset, but results in distance distortions.
	 */
	Location operator - (const Location &l) const {
		return Location(lon - l.lon, lat - l.lat);
	}
	/**
	 * Subtracts two Location objects together; useful for testing with a
	 * position offset, but results in distance distortions.
	 */
	Location &operator -= (const Location &l)  {
		lon -= l.lon;
		lat -= l.lat;
		return *this;
	}
};

/**
 * Returns the approximate distance in meters between two locations on Earth
 * given as longitude and latitude in degrees.
 */
double haversineEarth(const Location &l0, const Location &l1);

/**
 * Computes the azimuth and elevation of the sun for the given time and
 * location on Earth. The implementation is based on the text at:
 * https://www.esrl.noaa.gov/gmd/grad/solcalc/solareqns.PDF
 *
 * This isn't the most accurate computation, but I hope it'll be good enough
 * for me. The values for mid-totality for elevation are within a couple degrees
 * or so. Azimuth is only good somewhere east of longitude -93 degrees; I have
 * no idea why. Elevation values generally seem good, but azimuth values are bad
 * for anywhere and anytime other than east of -93 during totality.
 */
void sunPosition(
	double &azimuth,    // bad result
	double &elevation,  // usable result
	const Location &loc,
	const duds::time::interstellar::SecondTime &time
);

/**
 * Hours-minutes-seconds; needed for displaying time durations.
 */
struct Hms {
	int h, m, s;
	Hms() = default;
	Hms(int seconds) {
		set(seconds);
	}
	Hms(std::chrono::seconds seconds) {
		set(seconds.count());
	}
	void set(int seconds);
	void set(std::chrono::seconds seconds) {
		set(seconds.count());
	}
	void writeDuration(std::ostream &os) const;
	std::string duration() const;
	void writeTime(std::ostream &os) const;
	std::string time() const;
};
