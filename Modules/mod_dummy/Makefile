# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua ) -DLUA -DDEBUG -DMCHECK="mcheck(NULL)"

mod_dummy.o : mod_dummy.c mod_dummy.h ../Marcel/Module.h \
  ../Marcel/Section.h ../mod_Lua/mod_Lua.h Makefile 
	$(cc) -c -o mod_dummy.o mod_dummy.c $(opts) 

processStructDummy.o : processStructDummy.c mod_dummy.h \
  ../Marcel/Module.h ../Marcel/Section.h ../mod_Lua/mod_Lua.h Makefile 
	$(cc) -c -o processStructDummy.o processStructDummy.c $(opts) 

sectionEcho.o : sectionEcho.c mod_dummy.h ../Marcel/Module.h \
  ../Marcel/Section.h ../Marcel/MQTT_tools.h Makefile 
	$(cc) -c -o sectionEcho.o sectionEcho.c $(opts) 

../../mod_dummy.so : sectionEcho.o processStructDummy.o mod_dummy.o \
  Makefile 
	 $(cc) -shared -o ../../mod_dummy.so sectionEcho.o \
  processStructDummy.o mod_dummy.o $(opts) 

all: ../../mod_dummy.so 
