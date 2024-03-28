# Eclipse2024

A program to assist with photographing the April 8, 2024 total solar eclipse. It runs on a custom computer system made with a Raspberry Pi Zero, a GPS receiver, a graphic LCD, and a few other things. The code is not specific to the Raspberry Pi.

The program works out when totality, when the moon completely obstructs the sun, will occur for its location. This is done using data published by NASA that projects the irregular shape of the moon's umbra onto the irregular shape of the Earth at one second intervals. The programs should be able to tell when totality starts and ends to within one second. The NASA data is available at:

https://svs.gsfc.nasa.gov/5073

The umbra_hi files are the important ones. The program is told where to find the file using the --shape argument (use --help for more info).

The code here was written in a bit of a rush, so it isn't my best. It uses code from my earlier 2017 eclipse project and does have some architectural hold-overs.

# Missing Features

The program currently lacks a few things:
 - The start and end of the eclipse is computed as constant time offsets from totality based on where I expect to view the eclipse.

# Current Bugs and Issues

Testing is ongoing.
 - The buzzer makes more beeps than intended. The beeps before the event are correct, but it keeps beeping for a while longer.
 - The schedule page builds a list of events and schedules alarms when it is visited, but it would be better if it occured on demand. This likely won't be a problem during the eclipse, but it does mean that the page should be visited after setting a time offset for testing.
 - After setting the time offset, the offset must be set to zero before changing it to something else, or the offset will be incorrect.

# Dependencies

The following libraries are required:
 - GDAL
 - GEOS
 - [Boost](http://www.boost.org/)
 - GPSD's C++ library
 - evdev
 - [DUDS](https://github.com/jjackowski/duds)
   - The default location for the DUDS library is at the same directory level as wherever this code finds itself (../duds).

These programs are required:
 - gpsd
 - Some variation of ntpd
 - scons for the build
   - Run "scons -h" for build options.

# Hardware

The first two items use GPIO lines. Which ones are defined in the pins.conf file, or the file specified by the --conf argument. Several devices are connected by I2C; the program expects them to be on the same bus. The device file for the I2C bus is specified by the --i2cdev argument.

 - ST7920 display controller
   - Code expects a 144x32 resolution display, but the resolution can be changed in program arguments.
   - An argument is required to use the display (--st7920), otherwise simulated output will be sent to the console.
 - Buzzer (optional)
 - TSL2591 (optional)
   - Used to control PWM output for display backlight brighness control.
 - INA219 (optional)
   - Monitors battery status. The system is powered by 8 AA rechargeables.
 - AM2320 (optional)
   - Temperature and relative humidity just for fun.
 - VEML6070 (future; optional)
   - UV brightness just for fun.

# License

GPL v3
