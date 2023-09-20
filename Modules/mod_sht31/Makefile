# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua5.3 ) -DLUA  

mod_sht31.o : mod_sht31.c mod_sht31.h ../Marcel/Module.h \
  ../Marcel/Section.h ../mod_Lua/mod_Lua.h ../Marcel/MQTT_tools.h \
  Makefile 
	$(cc) -c -o mod_sht31.o mod_sht31.c $(opts) 

../../mod_sht31.so : mod_sht31.o Makefile 
	 $(cc) -shared -o ../../mod_sht31.so mod_sht31.o $(opts) 

all: ../../mod_sht31.so 
