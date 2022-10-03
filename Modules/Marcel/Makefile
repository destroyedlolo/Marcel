# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  9 2022 10:57:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC -DDEBUG -DMCHECK="mcheck(NULL)" $(shell pkg-config --libs lua5.3 ) 	-DPLUGIN_DIR='"/home/laurent/Projets/Marcel"' -L/home/laurent/Projets/Marcel 	-L/home/laurent/Projets/Marcel -lpaho-mqtt3c -lm -ldl -Wl,--export-dynamic -lpthread 	

MQTT_tools.o : MQTT_tools.c MQTT_tools.h Makefile 
	$(cc) -c -o MQTT_tools.o MQTT_tools.c $(opts) 

Marcel.o : Marcel.c Marcel.h Version.h Module.h Section.h mod_core.h \
  Makefile 
	$(cc) -c -o Marcel.o Marcel.c $(opts) 

Module.o : Module.c Module.h Marcel.h Section.h Makefile 
	$(cc) -c -o Module.o Module.c $(opts) 

Section.o : Section.c Section.h Marcel.h Makefile 
	$(cc) -c -o Section.o Section.c $(opts) 

helpers.o : helpers.c Marcel.h MQTT_tools.h Makefile 
	$(cc) -c -o helpers.o helpers.c $(opts) 

mod_core.o : mod_core.c mod_core.h Module.h Marcel.h Section.h \
  Makefile 
	$(cc) -c -o mod_core.o mod_core.c $(opts) 

../../Marcel : mod_core.o helpers.o Section.o Module.o Marcel.o \
  MQTT_tools.o Makefile 
	 $(cc) -o ../../Marcel mod_core.o helpers.o Section.o Module.o \
  Marcel.o MQTT_tools.o $(opts) 

all: ../../Marcel 
