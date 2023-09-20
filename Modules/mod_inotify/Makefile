# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua5.3 ) -DLUA  

mod_inotify.o : mod_inotify.c mod_inotify.h ../Marcel/Module.h \
  ../Marcel/Section.h ../Marcel/MQTT_tools.h ../mod_Lua/mod_Lua.h \
  Makefile 
	$(cc) -c -o mod_inotify.o mod_inotify.c $(opts) 

../../mod_inotify.so : mod_inotify.o Makefile 
	 $(cc) -shared -o ../../mod_inotify.so mod_inotify.o $(opts) \
  

all: ../../mod_inotify.so 
