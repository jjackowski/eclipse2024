# This file is part of the Eclipse2024 project. It is subject to the GPLv3
# license terms in the LICENSE file found in the top-level directory of this
# distribution and at
# https://github.com/jjackowski/eclipse2024/blob/master/LICENSE.
# No part of the Eclipse2024 project, including this file, may be copied,
# modified, propagated, or distributed except according to the terms
# contained in the LICENSE file.
#
# Copyright (C) 2024  Jeff Jackowski

import os
import platform
import subprocess

# CXX = 'distcc g++' might use distcc correctly

#####
# setup the build options
buildopts = Variables('localbuildconfig.py')
buildopts.Add('CCDBGFLAGS',
	'The flags to use with the compiler for debugging builds.',
	'-g -fno-common -O0 -Wno-psabi')
	#'-g -fno-common -Og')
	# -Og should be good for debugging, but too often it prevents important
	# data structures from being inspected by gdb
buildopts.Add('CCOPTFLAGS',
	'The flags to use with the compiler for optimized non-debugging builds.',
	'-O2 -ffunction-sections -fno-common -ffast-math -Wno-psabi')
buildopts.Add('LINKDBGFLAGS',
	'The flags to use with the linker for debugging builds.',
	'-rdynamic')
buildopts.Add('LINKOPTFLAGS',
	'The flags to use with the linker for optimized builds.',
	'-Wl,--gc-sections')
buildopts.Add(PathVariable('BOOSTINC',
	'The directory containing the Boost header files, or "." for the system default.',
	'.')) #, PathVariable.PathAccept))
buildopts.Add(PathVariable('BOOSTLIB',
	'The directory containing the Boost libraries, or "." for the system default.',
	'.')) #, PathVariable.PathAccept))
buildopts.Add('BOOSTTOOLSET',
	'The toolset tag for Boost libraries. Include a leading dash.',
	'')
#buildopts.Add('BOOSTABI',
#	'The ABI tag for Boost libraries. Include a leading dash.',
#	'')
buildopts.Add('BOOSTTAG',
	'Additional tags for Boost libraries. The libraries must support threading. Include leading dashes.',
	'') #'-mt')
buildopts.Add('BOOSTVER',
	'The version tag for Boost libraries. Include a leading dash.',
	'')
buildopts.Add(PathVariable('EIGENINC',
	'The directory containing the Eigen header files.',
	'/usr/include/eigen3/', PathVariable.PathAccept))
buildopts.Add(PathVariable('EVDEVINC',
	'The libevdev include path.',
	'/usr/include/libevdev-1.0', PathVariable.PathAccept))
buildopts.Add(PathVariable('DUDSSRC',
	'The directory containing the DUDS source code, if used.',
	#Dir.abspath('../duds'), PathVariable.PathAccept))
	os.path.abspath('../duds')))
#buildopts.Add(PathVariable('DUDSINC',
#	'The DUDS include path; required.',
#	'${DUDSSRC}/duds'))
#buildopts.Add(PathVariable('DUDSLIB',
#	'The DUDS library path; required.',
#	'${DUDSSRC}/bin/${PSYS}-${PARCH}-${BUILDTYPE}/lib', PathVariable.PathAccept))
#buildopts.Add(PathVariable('DUDSTOOLS',
#	'The DUDS tools path; required.',
#	'${DUDSSRC}/bin/${PSYS}-${PARCH}-${BUILDTYPE}/tools', PathVariable.PathAccept))
buildopts.Add('DUDSTOOLSBUILD',
	'The build type of the Duds tools to use; either dbg or opt.',
	'dbg')

puname = platform.uname()

#####
# create the template build environment
env = Environment(
	variables = buildopts,
	PSYS = puname[0].lower(),
	PARCH = puname[4].lower(),
	DUDSINC = '${DUDSSRC}',
	DUDSLIB = '${DUDSSRC}/bin/${PSYS}-${PARCH}-${BUILDTYPE}/lib',
	DUDSTOOLS = '${DUDSSRC}/bin/${PSYS}-${PARCH}-${DUDSTOOLSBUILD}/tools',
	BOOSTABI = '',
	#CC = 'distcc armv6j-hardfloat-linux-gnueabi-gcc',
	#CXX = 'distcc armv6j-hardfloat-linux-gnueabi-g++',
	# include paths
	CPPPATH = [
		#'$BOOSTINC',
		'$DUDSINC',
		'#/.',
		#'$EIGENINC',
	],
	# options passed to C and C++ (?) compiler
	CCFLAGS = [
		# flags always used with compiler
	],
	# options passed to C++ compiler
	CXXFLAGS = [
		'-std=gnu++17'  # allow gcc extentions to C++, like __int128
		#'-std=c++17'    # no GNU extensions, no 128-bit integer
	],
	# macros
	CPPDEFINES = [
	],
	# options passed to the linker; using gcc to link
	LINKFLAGS = [
		# flags always used with linker
	],
	LIBPATH = [
		'$BOOSTLIB',
		'$DUDSLIB',
	],
	LIBS = [  # required libraries
		# a boost library must be fisrt
		'libboost_date_time${BOOSTTOOLSET}${BOOSTTAG}${BOOSTABI}${BOOSTVER}',
		'libboost_system${BOOSTTOOLSET}${BOOSTTAG}${BOOSTABI}${BOOSTVER}',
		'libboost_program_options${BOOSTTOOLSET}${BOOSTTAG}${BOOSTABI}${BOOSTVER}',
		'pthread',
		'libduds',
		#'m',
		'libgps',
		'libevdev',
		'libgdal'
	]
)


#####
# DUDS tools

tools = {
	'bppic': env.File('${DUDSTOOLS}/bppic')
}

# bit-per-pixel image compiler

def BppiArcBuilder(target, source, env):
	return subprocess.call([
		tools['bppic'].path,
		source[0].path,
		'-a',
		target[0].path
	]) != 0
bppiArcBuilder = Builder(action = BppiArcBuilder,
	src_suffix = '.bppi', suffix = '.bppia')

def BppiArc(env, source):
	# build rule for the image archive
	target = env.BppiArcBuilder(source)
	# dependency on the image compiler
	env.Depends(target, tools['bppic'])
	return target

def BppiCppBuilder(target, source, env):
	return subprocess.call([
		tools['bppic'].path,
		source[0].path,
		'-c',
		target[0].path
	]) != 0
bppiCppBuilder = Builder(action = BppiCppBuilder,
	src_suffix = '.bppi', suffix = '.h')

def BppiCpp(env, source):
	# build rule for the image archive
	target = env.BppiCppBuilder(source)
	# dependency on the image compiler
	env.Depends(target, tools['bppic'])
	return target


env.Append(BUILDERS = {
	'BppiArcBuilder' : bppiArcBuilder,
	'BppiCppBuilder' : bppiCppBuilder,
})

env.AddMethod(BppiArc)
env.AddMethod(BppiCpp)

# filled in later
env['optionalLibs'] = { }

#####
# Debugging build enviornment
dbgenv = env.Clone(LIBS = [ ])  # no libraries; needed for library check
dbgenv.AppendUnique(
	CCFLAGS = '$CCDBGFLAGS',
	LINKFLAGS = '$LINKDBGFLAGS',
	BINAPPEND = '-dbg',
	BUILDTYPE = 'dbg',
)

# While SCons has a nice way of dealing with configurations, it doesn't
# automatically add things in a way that works for the different build
# enviornments here, unless the configuration is done once for each, which
# doesn't seem to be necessary.
# These libraries will be linked with a test program or will be deemed to not
# exist, so this doesn't work with header-only libraries.
optionalLibs = {
	# key is the macro, value is the library
	#'LIBBOOST_TEST' :
	#	'libboost_unit_test_framework${BOOSTTOOLSET}${BOOSTTAG}${BOOSTABI}${BOOSTVER}',
	#'LIBBOOST_PROGRAM_OPTIONS' :
	#	'libboost_program_options${BOOSTTOOLSET}${BOOSTTAG}${BOOSTABI}${BOOSTVER}',
}
# Similar to above, but only for the debug build.
optionalDbgLibs = { }

#####
# extra cleaning
if GetOption('clean'):
	Execute(Delete(Glob('*~') + [
		'BuildConfig.h',
		'config.log',
	] ))
	env['Use_Eigen'] = False
	dbgenv['Use_Eigen'] = False
	env['Use_Evdev'] = True
	dbgenv['Use_Evdev'] = True
	env['Use_GpioDevPort'] = True
	dbgenv['Use_GpioDevPort'] = True

#####
# configure the build
else:
	#####
	# Configuration for Boost libraries
	conf = Configure(dbgenv, config_h = 'BuildConfig.h',
		conf_dir = env.subst('.conf/${PSYS}-${PARCH}')) #, help=False)
	# check for debugging versions of the Boost libraries, then for a non-debug version
	dbgenv['BOOSTABI'] = '-gd'
	if not conf.CheckLib(dbgenv.subst(env['LIBS'][0]), language = 'C++', autoadd=0):
		dbgenv['BOOSTABI'] = '-d'
		if not conf.CheckLib(dbgenv.subst(env['LIBS'][0]), language = 'C++', autoadd=0):
			dbgenv['BOOSTABI'] = ''
			if not conf.CheckLib(dbgenv.subst(env['LIBS'][0]), language = 'C++', autoadd=0):
				print('No suitable Boost libraries could be found.')
				# really lame to not give help when there is a config problem,
				# but that is what is going to happen; either exit here or get
				# errors in the script, both of which result in no help.
				#if GetOption('help'):
				#	Help(buildopts.GenerateHelpText(env, cmp))
				#Exit(1)
				if not GetOption('help'):
					Exit(1)
	# check optional libraries
	remlibs = [ ]
	for mac, lib in iter(optionalLibs.items()):
		if conf.CheckLib(dbgenv.subst(lib), language = 'C++', autoadd=0):
			conf.Define('HAVE_' + mac, 1, 'optional library')
		else:
			remlibs.append(mac)
	for mac in remlibs:
		del optionalLibs[mac]
	#
	# Linking to python looks bothersome. The Boost libraries don't follow the
	# same conventions as the other Boost libraries, and not all systems seem
	# to use the documented naming convention in the Boost documentation. Maybe
	# this will be cleared up sometime after Boost 1.54.
	#
	# check for Boost's Python library
	#if conf.CheckLib(dbgenv.subst('libboost_python${BOOSTTOOLSET}${BOOSTTAG}-py${BOOSTABI}${BOOSTVER}'), language = 'C++', autoadd=1):
	#	dbgenv['USE_PYTHON'

	# Boost stacktrace (broken in 1.65.0, maybe earlier; fixed in 1.65.1)
	#    1.65.0 includes push_options.pp, but the file isn't installed.
	# Also, the dl library is required.
	dbgenv.Append(CPPDEFINES = 'BOOST_STACKTRACE_USE_ADDR2LINE')
	if conf.CheckCXXHeader('boost/stacktrace.hpp') and \
	conf.CheckLib('dl', language = 'C++', autoadd=0):
		optionalDbgLibs['DUDS_ERRORS_VERBOSE'] = 'dl'
	# remove the macro used for the test
	dbgenv['CPPDEFINES'] = env['CPPDEFINES']

	# Eigen
	dbgenv.Append(CPPPATH = '$EIGENINC')
	if conf.CheckCXXHeader('Eigen/Geometry'):
		env['Use_Eigen'] = True
		dbgenv['Use_Eigen'] = True
	else:
		env['Use_Eigen'] = False
		dbgenv['Use_Eigen'] = False
	# remove the Eigen path; only add where needed
	dbgenv['CPPPATH'] = env['CPPPATH']

	# Evdev
	dbgenv.Append(CPPPATH = '$EVDEVINC')
	if conf.CheckCHeader('libevdev/libevdev.h') and \
	conf.CheckLib('evdev', language = 'C', autoadd=0):
		env['Use_Evdev'] = True
		dbgenv['Use_Evdev'] = True
	else:
		env['Use_Evdev'] = False
		dbgenv['Use_Evdev'] = False
	# remove the Eigen path; only add where needed
	dbgenv['CPPPATH'] = env['CPPPATH']

	# 128-bit integer support
	
	# TODO  Get this from DUDS BuildConfig.h
	
	if conf.CheckType('__int128', language = 'C++'):
		conf.Define('HAVE_INT128', 1, 'A 128-bit integer type is available.')

	#env.ParseConfig('pkg-config --cflags --libs ')  # error case?

	dbgenv = conf.Finish()

# add back the libraries
dbgenv['LIBS'] = env['LIBS']
# put in additional debug libs
for mac, lib in iter(optionalDbgLibs.items()):
	dbgenv.Append(
		CPPDEFINES = mac,
		LIBS = lib
	)
	dbgenv['optionalLibs'][mac] = lib
# remove Boost unit test library; should only be added for test programs
if 'LIBBOOST_TEST' in optionalLibs:
	del optionalLibs['LIBBOOST_TEST']
	havetestlib = True
else:
	havetestlib = False

#####
# Optimized build enviornment
optenv = env.Clone()
optenv.AppendUnique(
	CCFLAGS = '$CCOPTFLAGS',
	CPPDEFINES = 'NDEBUG',
	LINKFLAGS = '$LINKOPTFLAGS',
	BINAPPEND = '',
	BUILDTYPE = 'opt'
)

#####
# main build

if not GetOption('help'):
	envs = [ dbgenv, optenv ]
	# image build
	imgs = SConscript('images/SConscript', exports = 'env tools', duplicate=0,
		variant_dir = env.subst('bin/images'))
	Alias('images', imgs)

	for env in envs:
		# add in optional libraries
		for mac, lib in iter(optionalLibs.items()):
			env.Append(
				#CPPDEFINES = ('HAVE_' + mac, 1),
				LIBS = lib
			)
			env['optionalLibs'][mac] = lib
		# build program(s)
		prg = SConscript('SConscript', exports = 'env imgs', duplicate=0,
			variant_dir = env.subst('bin/${PSYS}-${PARCH}-${BUILDTYPE}'))
		Alias('prog-' + env['BUILDTYPE'], prg)
		# test programs
		#if havetestlib: #'LIBBOOST_TEST' in optionalLibs:
		#	tests = SConscript('tests/SConscript', exports = 'env libs tools', duplicate=0,
		#		variant_dir = env.subst('bin/${PSYS}-${PARCH}-${BUILDTYPE}/tests'))
		#	Alias('tests-' + env['BUILDTYPE'], tests)

	if havetestlib:
		Alias('tests', 'tests-dbg')
	Default('prog-dbg')

#####
# setup help text for the build options
Help(buildopts.GenerateHelpText(env))
if GetOption('help'):
	print('Build target aliases:')
	print('  prog-dbg    - The program; debugging build. This is the default.')
	print('  prog-opt    - The program; optimized build.')
	print('  images      - All bit-per-pixel image archives.')
	#if havetestlib:
	#	print('  tests-dbg   - All unit test programs; debugging build.')
	#	print('  tests-opt   - All unit test programs; optimized build.')
	#	print('  tests       - Same as tests-dbg.')
