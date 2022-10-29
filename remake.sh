#!/bin/bash
# this script is used to create Makefiles

# =============
# Configuration
# =============
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

# Dead Publisher Dectector
BUILD_DPD=1

# SHT31 local probe
BUILD_SHT31=1

# 1 wire probes handling
BUILD_1WIRE=1

# File system notification
BUILD_INOTIFY=1

# Freebox v4/v5 figures
BUILD_FREEBOX=1

# Example plugin
# This one is strictly NO-USE. Its only purpose is to demonstrate how to build a plugin
BUILD_DUMMY=1

# Enable debugging messages
DEBUG=1

# MCHECK - check memory concistency (see glibc's mcheck())
MCHECK=1


# Where to put so plugins
# ---
# production
#PLUGIN_DIR=/usr/local/lib/Marcel
# for development
PLUGIN_DIR=$( pwd )

# -------------------------------------
#      END OF CONFIGURATION AREA
# DON'T MODIFY ANYTHING AFTER THIS LINE
# DON'T MODIFY ANYTHING AFTER THIS LINE
# DON'T MODIFY ANYTHING AFTER THIS LINE
# DON'T MODIFY ANYTHING AFTER THIS LINE
# -------------------------------------

# =======================
# Build MakeMaker's rules
# =======================

echo -e "\nSet build options\n=================\n"

CFLAGS="-Wall -O2 -fPIC"
RDIR=$( pwd )

if [ ${BUILD_LUA+x} ]; then
# 	Test Lua version (development purpose)
#	LUA_DIR=/home/laurent/Projets/lua-5.3.4/install
#	LUA="-isystem $LUA_DIR/include"
#	LUALIB="-L$LUA_DIR/lib"

# 	system Lua
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


# ===============
# Build Makefiles
# ===============

echo -e "\nBuild Makefiles\n===============\n"

echo "# global Makefile that is calling sub directories ones" > Makefile
echo >> Makefile
echo "gotoall: all" >> Makefile
echo >> Makefile

echo "# Clean previous builds sequels" >> Makefile
echo "clean:" >> Makefile
echo -e "\trm *.so" >> Makefile

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
if [ ${BUILD_DPD+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_dpd' >> Makefile
fi
if [ ${BUILD_SHT31+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_sht31' >> Makefile
fi
if [ ${BUILD_1WIRE+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_1wire' >> Makefile
fi
if [ ${BUILD_INOTIFY+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_inotify' >> Makefile
fi
if [ ${BUILD_FREEBOX+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_freebox' >> Makefile
fi
if [ ${BUILD_DUMMY+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_dummy' >> Makefile
fi
echo -e '\t$(MAKE) -C Modules/Marcel' >> Makefile

# =================
# Build all modules
# =================

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

if [ ${BUILD_INOTIFY+x} ]; then
	cd Modules/mod_inotify
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_inotify.so > Makefile
	cd ../..
fi

if [ ${BUILD_FREEBOX+x} ]; then
	cd Modules/mod_freebox
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_freebox.so > Makefile
	cd ../..
fi

if [ ${BUILD_DUMMY+x} ]; then
	cd Modules/mod_dummy
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_dummy.so > Makefile
	cd ../..
fi

# =================
# Build main source
# =================

cd Modules/Marcel

LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUALIB \
	-DPLUGIN_DIR='\"$PLUGIN_DIR\"' -L$PLUGIN_DIR \
	-L$RDIR -lpaho-mqtt3c -lm -ldl -Wl,--export-dynamic -lpthread \
	" *.c -t=../../Marcel > Makefile

#echo
#echo "Don't forget if you want to run it without installing first"
#echo export LD_LIBRARY_PATH=$PLUGIN_DIR:$LD_LIBRARY_PATH

