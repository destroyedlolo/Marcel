# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  9 2022 10:57:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua5.3 ) -DLUA -DDEBUG -DMCHECK="mcheck(NULL)" $(shell pkg-config --libs lua5.3 )

exposedFunction.o : exposedFunction.c mod_Lua.h ../Marcel/Module.h \
  ../Marcel/Version.h ../Marcel/MQTT_tools.h Makefile 
	$(cc) -c -o exposedFunction.o exposedFunction.c $(opts) 

mod_Lua.o : mod_Lua.c mod_Lua.h ../Marcel/Module.h Makefile 
	$(cc) -c -o mod_Lua.o mod_Lua.c $(opts) 

../../mod_Lua.so : mod_Lua.o exposedFunction.o Makefile 
	 $(cc) -shared -o ../../mod_Lua.so mod_Lua.o exposedFunction.o \
  $(opts) 

all: ../../mod_Lua.so 
