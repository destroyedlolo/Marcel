# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua ) -DLUA  

mod_outfile.o : mod_outfile.c mod_outfile.h ../Marcel/Module.h \
  ../Marcel/Section.h ../Marcel/MQTT_tools.h ../mod_Lua/mod_Lua.h \
  Makefile 
	$(cc) -c -o mod_outfile.o mod_outfile.c $(opts) 

../../mod_outfile.so : mod_outfile.o Makefile 
	 $(cc) -shared -o ../../mod_outfile.so mod_outfile.o $(opts) \
  

all: ../../mod_outfile.so 
