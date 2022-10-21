# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC  -DDEBUG -DMCHECK="mcheck(NULL)"

mod_dpd.o : mod_dpd.c mod_dpd.h ../Marcel/Module.h ../Marcel/Section.h \
  ../mod_Lua/mod_Lua.h ../Marcel/MQTT_tools.h Makefile 
	$(cc) -c -o mod_dpd.o mod_dpd.c $(opts) 

../../mod_dpd.so : mod_dpd.o Makefile 
	 $(cc) -shared -o ../../mod_dpd.so mod_dpd.o $(opts) 

all: ../../mod_dpd.so 
