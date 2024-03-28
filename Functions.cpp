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
#include "Functions.hpp"
#include <cmath>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <duds/time/planetary/Planetary.hpp>

double haversineEarth(const Location &l0, const Location &l1) {
	// need radians
	double lon0r = l0.lon * M_PI / 180.0;
	double lat0r = l0.lat * M_PI / 180.0;
	double lon1r = l1.lon * M_PI / 180.0;
	double lat1r = l1.lat * M_PI / 180.0;
	// will be squared
	double latsin = std::sin((lat1r - lat0r) / 2.0); // squigly
	double lonsin = std::sin((lon1r - lon0r) / 2.0); // upside-down y
	// big equation
	return 2.0 * 6365000.0 * std::asin(  // Earth radius approx 6365000m
		std::sqrt(
			latsin * latsin +
			std::cos(lat0r) * std::cos(lat1r) * lonsin * lonsin
		)
	);
}

// q-like squiggle, or circle with near vertical line through it, is latitude
// Why does phi have two distinctly different lower-case forms, and more
// importantly, why do websites that use phi insist on using both lower-case
// forms on the same page?
void sunPosition_old(
	double &azimuth,
	double &elevation,
	const Location &loc,
	const duds::time::interstellar::SecondTime &time
) {
	boost::gregorian::date date = duds::time::planetary::earth->date(time);
	int ls = duds::time::planetary::earth->leaps.leapSeconds(time).count();
	// based on https://www.esrl.noaa.gov/gmd/grad/solcalc/solareqns.PDF
	// start of 2017 is at 1483228827s after T0
	// 2017 has no leap seconds or days
	// 1704153627 for start of 2024
	double y = 2.0 * M_PI * (
		(double)(time.time_since_epoch().count() - 1704153627 - ls) /
		(double)(60 * 60 * 24 * 365) // one year
	);
	double eqtime = 229.18 * (0.000075 + 0.001868 * std::cos(y) -
		0.032077 * std::sin(y) - 0.014615 * cos(2.0 * y) -
		0.040849 * std::sin(2.0 * y)
	);
	double decl = 0.006918 - 0.399912 * std::cos(y) + 0.070257 * std::sin(y) -
		0.006758 * std::cos(2.0 * y) + 0.000907 * std::sin(2.0 * y) -
		0.002697 * std::cos(3.0 * y) + 0.00148 * std::sin(3.0 * y);
	// redone to avoid needing a timezone; it isn't like the sun will jump
	// around in the sky if you adjust your clock
	double ha = (  // will be in radians
		// seconds into the current day
		(double)(time.time_since_epoch().count() - (
			(date.day_of_year() - 1) * 60 * 60 * 24
		))
		// modify by longitude; seconds per degree
		+ loc.lon * 60.0 * 60.0 * 24.0 / 360.0 +
		// modify by the eqtime thingy
		eqtime * 60.0  // convert from minutes
	) / (60.0 * 60.0 * 24.0)  // fraction of day
	* 2.0 * M_PI - M_PI;  // angle
	double latrad = loc.lat * M_PI / 180.0;
	double coszen = std::sin(latrad) * std::sin(decl) + std::cos(latrad) *
		std::cos(decl) * std::cos(ha);
	double zenith = std::acos(coszen);
	elevation = ((M_PI / 2.0) - zenith) * 180.0 / M_PI;
	// above results *seem* ok, but next is bad
	azimuth = (M_PI - std::acos(
		-(std::sin(latrad) * coszen - std::sin(decl)) /
		(std::cos(latrad) * std::sin(zenith))
	)) * 180.0 / M_PI;
}

double radians(double d) {
	return d * M_PI / 180.0;
}

double degrees(double r) {
	return r * 180.0 / M_PI;
}

void sunPosition(
	double &azimuth,
	double &elevation,
	const Location &loc,
	const duds::time::interstellar::SecondTime &time
) {
	boost::gregorian::date date = duds::time::planetary::earth->date(time);
	// start of day time
	duds::time::interstellar::SecondTime dayStartTime;
	duds::time::planetary::earth->date(dayStartTime, date);
	// converted from a NOAA spreadsheet, including many conversions between
	// radians and degrees to avoid introducing errors
	// Time of current day as a fraction of how much of the day has past.
	double e2 = (double)(time - dayStartTime).count() / (60.0 * 60.0 * 24.0);
	// The spreadsheet handles dates like a day number. This is the number used
	// for the start of 1970, which is what this code uses for time zero.
	//const double dateT0 = 25569.0;
	// use above to compute the spreadsheet's version of the date
	//double d2 =
	// fractional Julian day
	double f2 = (double)date.day_number() - 0.5 + e2;
	double g2 = (f2 - 2451545.0) / 36525.0;
	double i2 = std::fmod(280.46646 + g2 * (36000.76983 + g2 * 0.0003032), 360.0);
	double j2 = 357.52911 + g2 * (35999.05029 - 0.0001537 * g2);
	double k2 = 0.016708634 - g2 * (0.000042037 + 0.0000001267 * g2);
	double l2 = std::sin(radians(j2)) * (1.914602 - g2
		* (0.004817 + 0.000014 * g2)) + std::sin(radians(2.0 * j2))
		* (0.019993 - 0.000101 * g2) + std::sin(radians(3.0 * j2)) * 0.000289;
	double m2 = i2 + l2;
	double p2 = m2 - 0.00569 - 0.00478 * std::sin(radians(125.04 - 1934.136 * g2));
	double q2 = 23.0 + (26.0 + ((21.448 - g2 * (46.815 + g2 *
		(0.00059 - g2 * 0.001813)))) / 60.0) / 60.0;
	double r2 = q2 + 0.00256 * std::cos(radians(125.04 - 1934.136 * g2));
	double t2 = degrees(std::asin(std::sin(radians(r2)) * std::sin(radians(p2))));
	double u2 = std::tan(radians(r2 / 2.0)) * std::tan(radians(r2 / 2.0));
	double v2 = 4.0 * degrees(u2 * std::sin(2.0 * radians(i2)) - 2.0 * k2
		* std::sin(radians(j2)) + 4.0 * k2 * u2 * std::sin(radians(j2))
		* std::cos(2.0 * radians(i2)) - 0.5 * u2 * u2 * std::sin(4.0 * radians(i2))
		- 1.25 * k2 * k2 * std::sin(2.0 * radians(j2)));
	double ab2 = std::fmod(e2 * 1440.0 + v2 + 4.0 * loc.lon, 1440.0);
	double ac2 = ((ab2 / 4.0) < 0) ? (ab2 / 4.0 + 180.0) : (ab2 / 4.0 - 180.0);
	double ad2 = degrees(std::acos(std::sin(radians(loc.lat))
		* std::sin(radians(t2)) + std::cos(radians(loc.lat))
		* std::cos(radians(t2)) * std::cos(radians(ac2))));
	elevation = 90.0 - ad2;
	if (ac2 > 0) {
		azimuth = std::fmod(degrees(std::acos(((std::sin(radians(loc.lat))
			* cos(radians(ad2))) - sin(radians(t2))) / (cos(radians(loc.lat))
			* sin(radians(ad2))))) + 180.0, 360.0);
	} else {
		azimuth = std::fmod(540.0 - degrees(acos(((sin(radians(loc.lat))
			* cos(radians(ad2))) - sin(radians(t2))) / (cos(radians(loc.lat))
			* sin(radians(ad2))))), 360.0);
	}
}

void Hms::set(int seconds) {
	h = seconds / 3600;
	m = (seconds / 60) - (h * 60);
	s = seconds % 60;
}

void Hms::writeDuration(std::ostream &os) const {
	os << std::right;
	if (h) {
		os << h << 'h' << std::setfill('0') << std::setw(2);
	}
	if (h || m) {
		os << m << 'm' << std::setfill('0') << std::setw(2);
	}
	os << s << std::left << std::setfill(' ') << 's';
}

std::string Hms::duration() const {
	std::ostringstream oss;
	writeDuration(oss);
	return oss.str();
}

void Hms::writeTime(std::ostream &os) const {
	os << std::right << std::setw(2) << h << ':' << std::setfill('0') <<
	std::setw(2) << m << ':' << std::setw(2) << s << std::setfill(' ');
}

std::string Hms::time() const {
	std::ostringstream oss;
	writeTime(oss);
	return oss.str();
}
