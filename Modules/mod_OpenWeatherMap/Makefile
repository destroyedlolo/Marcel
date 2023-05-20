# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua ) -DLUA $(shell pkg-config --cflags json-c )  

mod_owm.o : mod_owm.c mod_owm.h ../Marcel/Module.h ../Marcel/Section.h \
  Makefile 
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
