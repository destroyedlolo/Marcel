# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua ) -DLUA -DDEBUG -DMCHECK="mcheck(NULL)"

mod_ups.o : mod_ups.c mod_ups.h ../Marcel/Module.h ../Marcel/Section.h \
  ../Marcel/MQTT_tools.h ../mod_Lua/mod_Lua.h Makefile 
	$(cc) -c -o mod_ups.o mod_ups.c $(opts) 

../../mod_ups.so : mod_ups.o Makefile 
	 $(cc) -shared -o ../../mod_ups.so mod_ups.o $(opts) 

all: ../../mod_ups.so 
