# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua5.3 ) -DLUA -DDEBUG -DMCHECK="mcheck(NULL)"

mod_freeboxV5.o : mod_freeboxV5.c mod_freeboxV5.h ../Marcel/Module.h \
  ../Marcel/Section.h ../Marcel/MQTT_tools.h Makefile 
	$(cc) -c -o mod_freeboxV5.o mod_freeboxV5.c $(opts) 

../../mod_freeboxV5.so : mod_freeboxV5.o Makefile 
	 $(cc) -shared -o ../../mod_freeboxV5.so mod_freeboxV5.o \
  $(opts) 

all: ../../mod_freeboxV5.so 
