# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua5.3 ) -DLUA -DDEBUG -DMCHECK="mcheck(NULL)"

mod_freeboxOS.o : mod_freeboxOS.c mod_freeboxOS.h ../Marcel/Module.h \
  ../Marcel/Section.h ../Marcel/MQTT_tools.h Makefile 
	$(cc) -c -o mod_freeboxOS.o mod_freeboxOS.c $(opts) 

../../mod_freeboxOS.so : mod_freeboxOS.o Makefile 
	 $(cc) -shared -o ../../mod_freeboxOS.so mod_freeboxOS.o \
  $(opts) 

all: ../../mod_freeboxOS.so 
