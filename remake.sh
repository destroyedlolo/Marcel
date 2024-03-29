#!/bin/bash
# this script is used
# - enable / disable the build of particular modules
# - to create/update Makefiles

# ==================
# Configuration Area
# ==================
# Uncomment lines of stuffs to be enabled

# Lua callbacks and plugs-in
BUILD_LUA=1

if [ ${BUILD_LUA+x} ]; then
	# Repetitive task
	BUILD_EVERY=1
fi

# UPS / NUT server
BUILD_UPS=1

# Write to flat files
BUILD_OUTFILE=1

# Write to flat files
BUILD_ONOFF=1

# Dead Publisher Dectector
BUILD_DPD=1

# SHT31 local probe
BUILD_SHT31=1

# 1 wire probes handling
BUILD_1WIRE=1

# Alerting
BUILD_ALERT=1

# File system notification
BUILD_INOTIFY=1

# Meteo forcast using OpenWeatherMap
BUILD_METEOOWM=1

# Freebox v4/v5 figures
BUILD_FREEBOXV5=1

# FreeboxOS figures
# BUILD_FREEBOXOS=1

# RFXcom handling
BUIlD_RFXTRX=1

# Example plugin
# This one is strictly NO-USE. Its only purpose is to demonstrate how to build a plugin
BUILD_DUMMY=1

### 
# Development related
###

# Enable debugging messages
#DEBUG=1

# MCHECK - check memory concistency (see glibc's mcheck())
#MCHECK=1


# Where to generate ".so" plugins
# ---
# production's target directory
PLUGIN_DIR=/usr/local/lib/Marcel
# During development, being clean and keep everything in our own directory
#PLUGIN_DIR=$( pwd )

# -------------------------------------
#      END OF CONFIGURATION AREA
# DON'T MODIFY ANYTHING AFTER THIS LINE
# DON'T MODIFY ANYTHING AFTER THIS LINE
# DON'T MODIFY ANYTHING AFTER THIS LINE
# DON'T MODIFY ANYTHING AFTER THIS LINE
# -------------------------------------

# Error is fatal
set -e

# =============================
# Configure external components
# =============================

echo -e "\nSet build options\n=================\n"

CFLAGS="-Wall -O2 -fPIC"
RDIR=$( pwd )

if [ ${BUILD_LUA+x} ]; then
# Hardcode test Lua version
# Development purpose only or if pkg-config doesn't work
#
#	LUA_DIR=/home/laurent/Projets/lua-5.3.4/install
#	LUA="-isystem $LUA_DIR/include"
#	LUALIB="-L$LUA_DIR/lib"
#
# If used, uncomment the lines above and comment out system's Lua
# detection bellow.

# Find out system's installed Lua

	VERLUA=$( lua -v 2>&1 | grep -o -E '[0-9]\.[0-9]' )
	echo -n "Lua's version :" $VERLUA

	if pkg-config --cflags lua$VERLUA > /dev/null 2>&1; then
		echo "  (Packaged)"
		LUA="\$(shell pkg-config --cflags lua$VERLUA ) -DLUA"
		LUALIB="\$(shell pkg-config --libs lua$VERLUA )"
	elif pkg-config --cflags lua > /dev/null 2>&1; then
		echo " (unpackaged)"
		LUA="\$(shell pkg-config --cflags lua ) -DLUA"
		LUALIB="\$(shell pkg-config --libs lua )"
	else
		echo " - No package found"
		echo "Workaround : edit this remake file to hardcode where Lua is installed."
		echo
		exit 1
	fi

else
	echo "Lua not used"
fi

# Enable JSon-c for modules having to handle Json data as well as curl
if [ ${BUILD_METEOOWM+x} ]; then
	JSON="\$(shell pkg-config --cflags json-c )"
	JSONLIB="\$(shell pkg-config --libs json-c ) -lcurl"
else
	echo 'No need for Curl and json'
fi

if [ ${DEBUG+x} ]; then
	echo "Debuging code enabled"
	DEBUG="-DDEBUG"
else
	echo "DEBUG not defined"
fi

if [ ${MCHECK+x} ]; then
	echo "Memory checking activated"

	MCHECK='-DMCHECK="mcheck(NULL)"'
	MCHECK_LIB="-lmcheck"
else
	echo "No memory checking"
fi


# =================
# Rebuild Makefiles
# =================

echo -e "\nBuild Makefiles\n===============\n"

echo "# global Makefile that is calling sub directories ones" > Makefile
echo >> Makefile
echo "gotoall: all" >> Makefile
echo >> Makefile

echo "# Clean previous builds sequels" >> Makefile
echo "clean:" >> Makefile
echo -e "\trm -f *.so" >> Makefile
echo -e "\trm -f Modules/*/*.o" >> Makefile

echo >> Makefile
echo "# Build everything" >> Makefile
echo "all:" >> Makefile
if [ ${BUILD_LUA+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_Lua' >> Makefile
fi
if [ ${BUILD_EVERY+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_every' >> Makefile
fi
if [ ${BUILD_UPS+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_ups' >> Makefile
fi
if [ ${BUILD_OUTFILE+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_outfile' >> Makefile
fi
if [ ${BUILD_ONOFF+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_OnOff' >> Makefile
fi
if [ ${BUILD_DPD+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_dpd' >> Makefile
fi
if [ ${BUILD_SHT31+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_sht31' >> Makefile
fi
if [ ${BUILD_1WIRE+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_1wire' >> Makefile
fi
if [ ${BUILD_ALERT+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_alert' >> Makefile
fi
if [ ${BUILD_INOTIFY+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_inotify' >> Makefile
fi
if [ ${BUILD_METEOOWM+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_OpenWeatherMap' >> Makefile
fi
if [ ${BUILD_FREEBOXV5+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_freeboxV5' >> Makefile
fi
if [ ${BUILD_FREEBOXOS+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_freeboxOS' >> Makefile
fi
if [ ${BUIlD_RFXTRX+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_RFXtrx' >> Makefile
fi
if [ ${BUILD_DUMMY+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_dummy' >> Makefile
fi
echo -e '\t$(MAKE) -C Modules/Marcel' >> Makefile

# =============================
# Rebuild modules' own Makefile
# =============================

if [ ${BUILD_LUA+x} ]; then
	cd Modules/mod_Lua
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK $LUALIB" *.c -so=../../mod_Lua.so > Makefile
	cd ../..
fi

if [ ${BUILD_EVERY+x} ]; then
	cd Modules/mod_every
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_every.so > Makefile
	cd ../..
fi

if [ ${BUILD_UPS+x} ]; then
	cd Modules/mod_ups
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_ups.so > Makefile
	cd ../..
fi

if [ ${BUILD_OUTFILE+x} ]; then
	cd Modules/mod_outfile
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_outfile.so > Makefile
	cd ../..
fi

if [ ${BUILD_ONOFF+x} ]; then
	cd Modules/mod_OnOff
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_OnOff.so > Makefile
	cd ../..
fi

if [ ${BUILD_DPD+x} ]; then
	cd Modules/mod_dpd
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_dpd.so > Makefile
	cd ../..
fi

if [ ${BUILD_SHT31+x} ]; then
	cd Modules/mod_sht31
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_sht31.so > Makefile
	cd ../..
fi

if [ ${BUILD_1WIRE+x} ]; then
	cd Modules/mod_1wire
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_1wire.so > Makefile
	cd ../..
fi

if [ ${BUILD_ALERT+x} ]; then
	cd Modules/mod_alert
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_alert.so > Makefile
	cd ../..
fi
if [ ${BUILD_INOTIFY+x} ]; then
	cd Modules/mod_inotify
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_inotify.so > Makefile
	cd ../..
fi

if [ ${BUILD_METEOOWM+x} ]; then
	cd Modules/mod_OpenWeatherMap
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $JSON $DEBUG $MCHECK" *.c -so=../../mod_owm.so > Makefile
	cd ../..
fi

if [ ${BUILD_FREEBOXV5+x} ]; then
	cd Modules/mod_freeboxV5
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_freeboxV5.so > Makefile
	cd ../..
fi

if [ ${BUILD_FREEBOXOS+x} ]; then
	cd Modules/mod_freeboxOS
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_freeboxOS.so > Makefile
	cd ../..
fi

if [ ${BUIlD_RFXTRX+x} ]; then
	cd Modules/mod_RFXtrx
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_RFXtrx.so > Makefile
	cd ../..
fi

if [ ${BUILD_DUMMY+x} ]; then
	cd Modules/mod_dummy
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_dummy.so > Makefile
	cd ../..
fi

# ==============================
# Rebuild Marcel's core Makefile
# ==============================

cd Modules/Marcel

LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUALIB $JSONLIB \
	-DPLUGIN_DIR='\"$PLUGIN_DIR\"' -L$PLUGIN_DIR \
	-L$RDIR -lpaho-mqtt3c -lm -ldl -Wl,--export-dynamic -lpthread \
	" *.c -t=../../Marcel > Makefile

#echo
#echo "Don't forget if you want to run it without installing first"
#echo export LD_LIBRARY_PATH=$PLUGIN_DIR:$LD_LIBRARY_PATH

echo
echo "Makefiles are created"
