# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua5.3 ) -DLUA -DDEBUG -DMCHECK="mcheck(NULL)"

mod_RFXtrx.o : mod_RFXtrx.c mod_RFXtrx.h ../Marcel/Module.h \
  ../Marcel/Section.h ../mod_Lua/mod_Lua.h ../Marcel/MQTT_tools.h \
  RFXtrx.h Makefile 
	$(cc) -c -o mod_RFXtrx.o mod_RFXtrx.c $(opts) 

../../mod_RFXtrx.so : mod_RFXtrx.o Makefile 
	 $(cc) -shared -o ../../mod_RFXtrx.so mod_RFXtrx.o $(opts) 

all: ../../mod_RFXtrx.so 
