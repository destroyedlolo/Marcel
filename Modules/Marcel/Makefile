# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC -DDEBUG -DMCHECK="mcheck(NULL)" $(shell pkg-config --libs lua ) $(shell pkg-config --libs json-c ) -lcurl 	-DPLUGIN_DIR='"/home/laurent/Projets/Marcel"' -L/home/laurent/Projets/Marcel 	-L/home/laurent/Projets/Marcel -lpaho-mqtt3c -lm -ldl -Wl,--export-dynamic -lpthread 	

CURL_helpers.o : CURL_helpers.c CURL_helpers.h Marcel.h Makefile 
	$(cc) -c -o CURL_helpers.o CURL_helpers.c $(opts) 

helpers.o : helpers.c Marcel.h MQTT_tools.h Makefile 
	$(cc) -c -o helpers.o helpers.c $(opts) 

Marcel.o : Marcel.c Marcel.h Version.h Module.h Section.h mod_core.h \
  Makefile 
	$(cc) -c -o Marcel.o Marcel.c $(opts) 

mod_core.o : mod_core.c mod_core.h Module.h Marcel.h Section.h \
  Makefile 
	$(cc) -c -o mod_core.o mod_core.c $(opts) 

Module.o : Module.c Module.h Marcel.h Section.h Makefile 
	$(cc) -c -o Module.o Module.c $(opts) 

MQTT_tools.o : MQTT_tools.c MQTT_tools.h Makefile 
	$(cc) -c -o MQTT_tools.o MQTT_tools.c $(opts) 

Section.o : Section.c Section.h Marcel.h Makefile 
	$(cc) -c -o Section.o Section.c $(opts) 

../../Marcel : Section.o MQTT_tools.o Module.o mod_core.o Marcel.o \
  helpers.o CURL_helpers.o Makefile 
	 $(cc) -o ../../Marcel Section.o MQTT_tools.o Module.o \
  mod_core.o Marcel.o helpers.o CURL_helpers.o $(opts) 

all: ../../Marcel 
