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
/**
 * @file
 * The main() function for a program to assit with photographing the 2024 total
 * solar eclipse in North America.
 * @author  Jeff Jackowski
 */

#include <duds/hardware/devices/displays/SimulatedBppDisplay.hpp>
#include <duds/hardware/devices/displays/ST7920.hpp>
#include <duds/hardware/interface/linux/GpioDevPort.hpp>
#include <duds/hardware/interface/PinConfiguration.hpp>
#include <duds/hardware/devices/clocks/LinuxClock.hpp>
#include <duds/hardware/devices/instruments/AM2320.hpp>
#include <duds/hardware/devices/instruments/INA219.hpp>
#include <duds/hardware/devices/instruments/TSL2591.hpp>
#include <duds/hardware/interface/linux/DevSmbus.hpp>
#include <duds/hardware/interface/SmbusErrors.hpp>
#include <duds/hardware/interface/linux/DevI2c.hpp>
#include <duds/hardware/interface/I2cErrors.hpp>
#include <duds/hardware/interface/linux/SysPwm.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <duds/ui/graphics/BppFontPool.hpp>
#include <iostream>
#include <assert.h>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/string_generator.hpp>
#include <future>
#include <csignal>
#include <libgpsmm.h>
#include "RunUi.hpp"
#include "Umbra.hpp"

/**
 * Available fonts:
 * | Name         | Height | Image archive file
 * |--------------|-------:|--------------------
 * | Date-small   | 8      | font_Vx8.bppia
 * | Date-medium  | 8      | font_Vx8B.bppia
 * | Date-large   | 18     | font_10x18.bppia
 * | Time-tiny    | 8      | font_Vx8.bppia
 * | Time-small   | 8      | font_Vx8B.bppia
 * | Time-medium  | 14     | font_7x14.bppia (could make reduced height version)
 * | Time-large   | 17     | timefont_12x17.bppia
 */
duds::ui::graphics::BppFontPool FontPool;

duds::hardware::devices::clocks::LinuxClockSptr Clock;

static DisplayStuff displaystuff;

std::atomic_bool quit(false);

void signalHandler(int) {
	quit = true;
}

void check(Umbra &umbra, const Location &loc)
try {
	bool res = umbra.check(loc.lon, loc.lat);
	displaystuff.updateTotality(umbra.startTime(), umbra.endTime(), res);
} catch (...) {
	std::cerr << "Program failed in umbra check thread:\n" <<
	boost::current_exception_diagnostic_information()
	<< std::endl;
}

int main(int argc, char *argv[])
try {
	std::string fontpath, confpath, lcdname, shapepath, zonepath, i2cpath;
	std::string imgpath(argv[0]), extimgpath;
	double tlon = 400.0, tlat = 400.0;
	int dispW, dispH;
	bool uselcd = false;
	{
		int found = 0;
		while (!imgpath.empty() && (found < 3)) {
			imgpath.pop_back();
			if (imgpath.back() == '/') {
				++found;
			}
		}
		extimgpath = imgpath;
		imgpath += "bin/images/";
		while (!extimgpath.empty() && (found < 4)) {
			extimgpath.pop_back();
			if (extimgpath.back() == '/') {
				++found;
			}
		}
		if (extimgpath.empty()) {
			extimgpath = "../";
		}
		extimgpath += "duds/bin/images/";
	}
	{ // option parsing
		boost::program_options::options_description optdesc(
			"Options for clock test program"
		);
		optdesc.add_options()
			( // help info
				"help,h",
				"Show this help message"
			)
			(
				"imagedir,d",
				boost::program_options::value<std::string>(&imgpath)->
					default_value(imgpath),
				"Image path"
			)
			(
				"extimagedir,e",
				boost::program_options::value<std::string>(&extimgpath)->
					default_value(extimgpath),
				"External image path"
			)
			(
				"st7920",
				"Use a graphic ST7920 LCD instead of console output"
			)
			(
				"conf,c",
				boost::program_options::value<std::string>(&confpath)->
					default_value("pins.conf"),
				"Pin configuration file"
			)
			(
				"i2cdev,i",
				boost::program_options::value<std::string>(&i2cpath)->
					default_value("/dev/i2c-1"),
				"Specify I2C device file"
			)
			(
				"lcdname",
				boost::program_options::value<std::string>(&lcdname)->
					default_value("lcdGraphic"),
				"Name of LCD inside pin configuration"
			)
			( // the display width
				"width,x",
				boost::program_options::value<int>(&dispW)->
					default_value(144),
				"ST7920 display width in pixels"
			)
			( // the display height
				"height,y",
				boost::program_options::value<int>(&dispH)->
					default_value(32),
				"ST7920 display height in pixels"
			)
			(
				"shape",
				boost::program_options::value<std::string>(&shapepath)->
					default_value("../umbra_hi.shp"),
				"Path to shapefile"
			)
			(
				"zone,z",
				boost::program_options::value<std::string>(&zonepath)->
					default_value("/usr/share/zoneinfo-leaps/UTC"),
				"Path to zoneinfo file with leap seconds"
			)
			(
				"lon",
				boost::program_options::value<double>(&tlon)->
					default_value(tlon),
				"Use the given longitude for testing; prevents use of gpsd"
			)
			(
				"lat",
				boost::program_options::value<double>(&tlat)->
					default_value(tlat),
				"Use the given latitude for testing; prevents use of gpsd"
			)
		;
		boost::program_options::variables_map vm;
		boost::program_options::store(
			boost::program_options::parse_command_line(argc, argv, optdesc),
			vm
		);
		boost::program_options::notify(vm);
		if (vm.count("help")) {
			std::cout << "Clock test program.\n\t" << argv[0] << " [options]\n"
			<< optdesc << std::endl;
			return 0;
		}
		if (vm.count("st7920")) {
			uselcd = true;
		}
	}
	Umbra umbra(shapepath, tlon < 200.0);
	// distant initial location helps ensure an early totality check
	Location prev(0.0, 0.0), curr;
	std::unique_ptr<gpsmm> gps;
	// use test location?
	if ((tlon < 200.0) || (tlat < 200.0)) {
		// configure test data
		displaystuff.setTesting();
		curr = Location(tlon, tlat);
		displaystuff.setCurrLoc(curr, 16, 8);
		displaystuff.setCheckLoc(curr);
		// start computing total eclipse length
		/*
		eclipseCalc = std::async(
			std::launch::async,
			&check,
			std::ref(umbra),
			std::ref(curr)
		);
		*/
		check(umbra, curr);
		DisplayInfo di;
		displaystuff.getInfo(di);
		if (di.inTotality) {
			std::cout << "In";
		} else {
			std::cout << "Out";
		}
		std::cout << "side area of totality." << std::endl;
	} else {
		// attempt to connect to GPSD
		gps = std::make_unique<gpsmm>("localhost", DEFAULT_GPSD_PORT);
		// failed?
		if (!gps->stream(WATCH_ENABLE|WATCH_JSON)) {
			gps.reset();
			displaystuff.setError("No gpsd connection", 8);
		}
	}
	// signal handler
	std::signal(SIGINT, &signalHandler);
	std::signal(SIGTERM, &signalHandler);

	// load fonts
	FontPool.addWithCache("Date-small", extimgpath + "/font_Vx8.bppia");
	FontPool.addWithCache("Date-medium", extimgpath + "/font_Vx8B.bppia");
	//FontPool.addWithCache("Date-large", extimgpath + "/font_10x18.bppia");
	//FontPool.addWithCache("Time-small", extimgpath + "/font_Vx7.bppia");
	//FontPool.alias("Date-small", "Time-medium");
	FontPool.alias("Date-small", "Time-tiny");
	FontPool.alias("Date-medium", "Time-small");
	FontPool.addWithCache("Time-medium", extimgpath + "/font_7x14.bppia");
	FontPool.addWithCache("Time-large", imgpath + "/timefont_12x17.bppia");
	FontPool.alias("Date-small", "Title");
	//FontPool.alias("Date-medium", "Title");
	FontPool.alias("Date-medium", "Text");
	// load icons
	duds::ui::graphics::BppImageArchiveSptr iconArc =
		duds::ui::graphics::BppImageArchive::make(imgpath + "/icons.bppia");

	std::shared_ptr<duds::hardware::devices::displays::SimulatedBppDisplay> sd;
	duds::hardware::display::BppGraphicDisplaySptr disp;
	// display configuration
	duds::hardware::interface::PinConfiguration pc;
	std::shared_ptr<duds::hardware::interface::DigitalPort> port;
	duds::hardware::interface::DigitalPin buzzer;
	// ST7920
	if (uselcd) {
		boost::property_tree::ptree tree;
		boost::property_tree::read_info(confpath, tree);
		// if an exception is thrown here, the program will terminate without
		// getting to the catch block below; don't know why
		pc.parse(tree.get_child("pins"));
		port = duds::hardware::interface::linux::GpioDevPort::makeConfiguredPort(pc);
		duds::hardware::interface::DigitalPinSet lcdset;
		duds::hardware::interface::ChipSelect lcdsel;
		pc.getPinSetAndSelect(lcdset, lcdsel, lcdname);
		std::shared_ptr<duds::hardware::devices::displays::ST7920> lcd =
			std::make_shared<duds::hardware::devices::displays::ST7920>(
				std::move(lcdset), std::move(lcdsel), dispW, dispH
			);
		lcd->initialize();
		disp = lcd;
		// also config other hardware using GPIO
		pc.getPin(buzzer, "buzzer");
	} else {
		disp = sd = std::make_shared<duds::hardware::devices::displays::SimulatedBppDisplay>(
			dispW, dispH
		);
		//disp = sd;
	}
	Clock = duds::hardware::devices::clocks::LinuxClock::make();
	duds::time::planetary::Earth::make(zonepath);

	// battery monitor
	std::unique_ptr<duds::hardware::devices::instruments::INA219> batmon;
	try {
		std::unique_ptr<duds::hardware::interface::Smbus> smbus(
			new duds::hardware::interface::linux::DevSmbus(
				i2cpath,
				0x40,
				duds::hardware::interface::Smbus::NoPec()
			)
		);
		batmon = std::make_unique<duds::hardware::devices::instruments::INA219>(
			smbus, 0.1
		);
	} catch (duds::hardware::interface::SmbusErrorNoBus &) {
		std::cerr << "ERROR: " << i2cpath << " cannot be opened." << std::endl;
	} catch (duds::hardware::interface::SmbusErrorNoDevice &) {
		std::cerr << "ERROR: The INA219 is not found." << std::endl;
	} catch (duds::hardware::interface::SmbusError &) {
		std::cerr << "ERROR: An error prevented using the INA219:\n" <<
		boost::current_exception_diagnostic_information() << std::endl;
	}
	// intialize exponential moving average for battery
	if (batmon) {
		batmon->sample();
		displaystuff.initBatteryData(batmon->busVoltage(), batmon->busPower());
	}
	// temperature monitor
	std::unique_ptr<duds::hardware::devices::instruments::AM2320> tempmon;
	// the AM2320 can be tempermental
	for (int t = 8; t > 0; --t) {
		try {
			std::unique_ptr<duds::hardware::interface::I2c> i2c(
				new duds::hardware::interface::linux::DevI2c(i2cpath, 0x5C)
			);
			tempmon = std::make_unique<duds::hardware::devices::instruments::AM2320>(
				i2c
			);
			break;
		} catch (duds::hardware::interface::I2cErrorNoBus &) {
			if (t == 1) {
				std::cerr << "ERROR: " << i2cpath << " cannot be opened."
				<< std::endl;
			} else {
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		} catch (duds::hardware::interface::I2cErrorNoDevice &) {
			if (t == 1) {
				std::cerr << "ERROR: The AM2320 is not found." << std::endl;
			} else {
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		} catch (duds::hardware::interface::I2cError &) {
			if (t == 1) {
				std::cerr << "ERROR: An error prevented using the AM2320:\n" <<
				boost::current_exception_diagnostic_information() << std::endl;
			} else {
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		}
	}
	// intialize exponential moving average for temperature & humidity
	if (tempmon) {
		// the AM2320 can be tempermental
		for (int t = 8; t > 0; --t) {
			try {
				std::this_thread::sleep_for(std::chrono::seconds(1));
				tempmon->sample();
				displaystuff.initTempData(
					tempmon->temperature(),
					tempmon->relHumidity()
				);
				break;
			} catch (...) { }
		}
	}
	// brightness sensor
	std::unique_ptr<duds::hardware::devices::instruments::TSL2591> brightmon;
	double brightness = 0;
	try {
		std::unique_ptr<duds::hardware::interface::I2c> i2c(
			new duds::hardware::interface::linux::DevI2c(i2cpath, 0x29)
		);
		brightmon = std::make_unique<duds::hardware::devices::instruments::TSL2591>(
			i2c
		);
		brightmon->init(0, 0);
		std::this_thread::sleep_for(std::chrono::milliseconds(128));
		brightness = (double)brightmon->brightnessCount();
	} catch (duds::hardware::interface::I2cErrorNoBus &) {
		std::cerr << "ERROR: " << i2cpath << " cannot be opened." << std::endl;
	} catch (duds::hardware::interface::I2cErrorNoDevice &) {
		std::cerr << "ERROR: The TSL2591 is not found." << std::endl;
	} catch (duds::hardware::interface::I2cError &) {
		std::cerr << "ERROR: An error prevented using the TSL2591:\n" <<
		boost::current_exception_diagnostic_information() << std::endl;
	}
	// PWM output - LCD brighness control
	std::unique_ptr<duds::hardware::interface::linux::SysPwm> pwmout;
	try {
		pwmout = std::make_unique<duds::hardware::interface::linux::SysPwm>(0, 0);
		pwmout->frequency(400);
		pwmout->dutyZero();
		pwmout->disable();
	} catch (...) {
		std::cerr << "ERROR: No PWM output." << std::endl;
		// no need for brightness
		brightmon.reset();
	}
	// make user interface
	RunUi ui(std::move(disp), std::move(iconArc), Clock, displaystuff, buzzer);
	if (!ui.initInput() && !displaystuff.isTesting()) {
		std::cerr << "ERROR: Failed to initialize input" << std::endl;
		displaystuff.setError("Missing input", 16);
	}
	// start user interface thread
	std::thread displayThread(&RunUi::run, std::ref(ui));

	// claim that the last totality check was 1 minute ago
	auto lastCheck = std::chrono::system_clock::now() - std::chrono::minutes(1);
	auto sampleTime = lastCheck;
	std::future<void> eclipseCalc;
	double speed = 0;  // in m/s
	gps_data_t *gpsInfo;

	// until signal requests termination
	while (!quit) {
		if (!displaystuff.isTesting()) {
			if (gps) {
				// up to 5 second delay
				if (!gps->waiting(5000000)) {
					continue;
				}
				gpsInfo = gps->read();
			}
			// failed?
			if (!gps || !gpsInfo) {
				speed = 0;
				// attempt to re-establish contact with gpsd
				gps.reset();
				do {
					displaystuff.setError("No gpsd connection", 8);
					gps = std::unique_ptr<gpsmm>(
						new gpsmm("localhost", DEFAULT_GPSD_PORT)
					);
					if (!gps->stream(WATCH_ENABLE|WATCH_JSON)) {
						gps.reset();
						std::this_thread::sleep_for(std::chrono::seconds(2));
					}
				} while (!gps && !quit);
				displaystuff.clearError();
			} else if (gps) {
				// gps_data_t::status moved to gps_fix_t
				if (                                  // with no satellite signals,
					(gpsInfo->fix.status == 0) ||     // the first two conditionals
					(gpsInfo->fix.mode != MODE_3D) || // usually are false, so 3D
					(gpsInfo->satellites_used == 0)   // fix with no satillites is
				) {                                   // possible?
					displaystuff.badFix();
				}
				// keep exponential moving average of speed for deciding when to
				// recalculate times of totality
				if (gpsInfo->set & SPEED_SET) {
					speed = 0.8 * speed + 0.2 * gpsInfo->fix.speed;
				}
				if (gpsInfo->set & LATLON_SET) {
					auto now = std::chrono::system_clock::now();
					auto diff = now - lastCheck;
					if (displaystuff.wasGood()) {
						curr.lon = curr.lon * (1.0 - 0.5 / gpsInfo->fix.epx) +
							0.5 / gpsInfo->fix.epx * gpsInfo->fix.longitude;
						curr.lat = curr.lat * (1.0 - 0.5 / gpsInfo->fix.epy) +
							0.5 / gpsInfo->fix.epy * gpsInfo->fix.latitude;
					}
					// protect against the position values going bad (NaN)
					if (!displaystuff.wasGood() || !std::isnormal(curr.lon) ||
						!std::isnormal(curr.lat)
					) {
						curr.lon = gpsInfo->fix.longitude;
						curr.lat = gpsInfo->fix.latitude;
					}
					displaystuff.setCurrLoc(
						curr ,
						(int)std::max(gpsInfo->fix.epy, gpsInfo->fix.epx),
						gpsInfo->satellites_used
					);
					/** @todo  Do not check for totality after totality. */
					// take into account a position offset (curr with off)
					Location cwo = curr + displaystuff.getLocOffset();
					double dist = haversineEarth(prev, cwo);
					// do not recompute too often, unless a large change in
					// position occurred
					if ((diff > std::chrono::seconds(128)) || (dist > 1024.0)) {
						// if the distance has changed by more than 64m and the
						// speed is low . . .
						if ((speed < 2.5) && (dist > 64.0)) {
							lastCheck = now;
							prev = cwo;
							displaystuff.setCheckLoc(curr);
							// start computing total eclipse length
							eclipseCalc = std::async(
								std::launch::async,
								&check,
								std::ref(umbra),
								std::ref(cwo)
							);
							// *
							std::cout << "Starting check " <<
							std::chrono::duration_cast<std::chrono::seconds>(diff).count()
							<< "s after last, dist = " << dist << std::endl;
							// */
						}
					}
				}
			} else {
				displaystuff.badFix();
			}
		} else {
			// testing, so sleep while waiting to quit
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}
		// sample sensors
		auto now = std::chrono::system_clock::now();
		if ((now - sampleTime) >= std::chrono::seconds(2)) {
			sampleTime = now;
			if (batmon) {
				try {
					batmon->sample();
					displaystuff.setBatteryData(
						batmon->busVoltage(),
						batmon->busPower()
					);
				} catch (...) { }
			}
			if (tempmon) {
				try {
					tempmon->sample();
					displaystuff.setTempData(
						tempmon->temperature(),
						tempmon->relHumidity()
					);
				} catch (...) { }
			}
		}
		// brighness is always sampled
		if (brightmon) {
			try {
				brightmon->sample();
			} catch (...) {
				try {
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					brightmon->init(0, 0);
				} catch (...) {
					brightmon.reset();
					pwmout->dutyCycle(0.4);
					pwmout->enable();
				}
			}
			// may have been destroyed above
			if (brightmon) {
				brightness = brightness * 0.8 +
					0.2 * (double)brightmon->brightnessCount();
				// plenty bright?
				if (brightness > 12000.0) {
					// no need of backlight
					pwmout->disable();
				}
				// somewhat bright
				else if (brightness > 8777.0) {
					// max backlight
					pwmout->dutyFull();
					pwmout->enable();
				}
				// dim
				else {
					// minimum backlight out of 10% at 1000 (not really) and below.
					/**
					 * @bug  If an exception is thrown in the next line, the
					 *       process will abort even though this is inside
					 *       a try-catch block that should catch any exception
					 *       type.
					 */
					pwmout->dutyCycle(std::max((brightness - 1000.0) / (7000.0 / 0.9), 0.08));
					pwmout->enable();
				}
			}
		}
	}
	// wait for threads to end
	try {
		eclipseCalc.get();
	} catch (...) {
		// may not have run the thread; ignore
	}
	try {
		// doesn't seem thread-safe, but next line hangs process without it
		if (displayThread.joinable()) {
			displayThread.join();
		}
	} catch (const std::system_error& e) {
		// invalid_argument may occur if thread has terminated
		if (e.code() != std::errc::invalid_argument) {
			// something else
			throw;
		}
	}
	//std::cout << std::endl;
	return 0;
} catch (...) {
	std::cerr << "Program failed in main():\n" <<
	boost::current_exception_diagnostic_information() << std::endl;
	return 1;
}
