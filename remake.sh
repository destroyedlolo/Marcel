#!/bin/bash
# this script is used to create Makefiles

# =============
# Configuration
# =============
# Uncomment lines of stuffs to be enabled

# Lua callbacks and plugs-in
BUILD_LUA=1

# Example plugin
# This one is strictly NO-USE. Its only purpose is to demonstrate how to build a plugin
BUILD_TEST=1

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
	echo -e '\t$(MAKE) -C src/mod_Lua' >> Makefile
fi
if [ ${BUILD_TEST+x} ]; then
	echo -e '\t$(MAKE) -C src/mod_test' >> Makefile
fi
echo -e '\t$(MAKE) -C src' >> Makefile

# =================
# Build all modules
# =================

if [ ${BUILD_LUA+x} ]; then
	cd src/mod_Lua
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_Lua.so > Makefile
	cd ../..
fi

if [ ${BUILD_TEST+x} ]; then
	cd src/mod_test
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_test.so > Makefile
	cd ../..
fi

# =================
# Build main source
# =================

cd src

LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK $LUALIB \
	-DPLUGIN_DIR='\"$PLUGIN_DIR\"' -L$PLUGIN_DIR \
	-L$RDIR -lpaho-mqtt3c -lm -ldl -Wl,--export-dynamic -lpthread \
	" *.c -t=../Marcel > Makefile

echo
echo "Don't forget if you want to run it without installing first"
echo export LD_LIBRARY_PATH=$PLUGIN_DIR:$LD_LIBRARY_PATH

