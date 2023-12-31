# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua ) -DLUA   $(shell pkg-config --libs lua )

exposedFunction.o : exposedFunction.c mod_Lua.h ../Marcel/Module.h \
  ../Marcel/Section.h ../Marcel/Version.h ../Marcel/MQTT_tools.h \
  mlSection.h Makefile 
	$(cc) -c -o exposedFunction.o exposedFunction.c $(opts) 

mlSection.o : mlSection.c mod_Lua.h ../Marcel/Module.h \
  ../Marcel/Section.h mlSection.h Makefile 
	$(cc) -c -o mlSection.o mlSection.c $(opts) 

mod_Lua.o : mod_Lua.c mod_Lua.h ../Marcel/Module.h ../Marcel/Section.h \
  mlSection.h Makefile 
	$(cc) -c -o mod_Lua.o mod_Lua.c $(opts) 

../../mod_Lua.so : mod_Lua.o mlSection.o exposedFunction.o Makefile 
	 $(cc) -shared -o ../../mod_Lua.so mod_Lua.o mlSection.o \
  exposedFunction.o $(opts) 

all: ../../mod_Lua.so 
