#!/bin/bash
# this script is used to create Makefiles

# =============
# Configuration
# =============
# Uncomment lines of stuffs to be enabled

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

if [ ${DEBUG+x} ]; then
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
if [ ${BUILD_TEST+x} ]; then
	echo -e '\t$(MAKE) -C src/mod_test' >> Makefile
fi
echo -e '\t$(MAKE) -C src' >> Makefile

# =================
# Build all modules
# =================

if [ ${BUILD_TEST+x} ]; then
	cd src/mod_test
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK" *.c -so=../../mod_test.so > Makefile
	cd ../..
fi

cd src

LFMakeMaker -v +f=Makefile --opts="$CFLAGS $DEBUG $MCHECK \
	-DPLUGIN_DIR='\"$PLUGIN_DIR\"' -L$PLUGIN_DIR \
	-L$RDIR -lpaho-mqtt3c $LUA -lm -ldl -Wl,--export-dynamic -lpthread \
	" *.c -t=../Marcel > Makefile

echo
echo "Don't forget if you want to run it without installing first"
echo export LD_LIBRARY_PATH=$PLUGIN_DIR:$LD_LIBRARY_PATH

