# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua5.3 ) -DLUA $(shell pkg-config --cflags json-c ) -DDEBUG -DMCHECK="mcheck(NULL)"

mod_owm.o : mod_owm.c mod_owm.h ../Marcel/Module.h ../Marcel/Section.h \
  ../mod_Lua/mod_Lua.h Makefile 
	$(cc) -c -o mod_owm.o mod_owm.c $(opts) 

Weather3H.o : Weather3H.c mod_owm.h ../Marcel/Module.h \
  ../Marcel/Section.h ../Marcel/MQTT_tools.h ../Marcel/Version.h \
  ../Marcel/CURL_helpers.h Makefile 
	$(cc) -c -o Weather3H.o Weather3H.c $(opts) 

WeatherDaily.o : WeatherDaily.c mod_owm.h ../Marcel/Module.h \
  ../Marcel/Section.h ../Marcel/MQTT_tools.h ../Marcel/Version.h \
  ../Marcel/CURL_helpers.h Makefile 
	$(cc) -c -o WeatherDaily.o WeatherDaily.c $(opts) 

../../mod_owm.so : WeatherDaily.o Weather3H.o mod_owm.o Makefile 
	 $(cc) -shared -o ../../mod_owm.so WeatherDaily.o Weather3H.o \
  mod_owm.o $(opts) 

all: ../../mod_owm.so 
