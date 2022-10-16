# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua ) -DLUA -DDEBUG -DMCHECK="mcheck(NULL)"

FFV.o : FFV.c mod_1wire.h ../Marcel/Module.h ../Marcel/Section.h \
  ../mod_Lua/mod_Lua.h ../Marcel/MQTT_tools.h Makefile 
	$(cc) -c -o FFV.o FFV.c $(opts) 

mod_1wire.o : mod_1wire.c mod_1wire.h ../Marcel/Module.h \
  ../Marcel/Section.h ../mod_Lua/mod_Lua.h ../Marcel/MQTT_tools.h \
  Makefile 
	$(cc) -c -o mod_1wire.o mod_1wire.c $(opts) 

../../mod_1wire.so : mod_1wire.o FFV.o Makefile 
	 $(cc) -shared -o ../../mod_1wire.so mod_1wire.o FFV.o $(opts) \
  

all: ../../mod_1wire.so 
