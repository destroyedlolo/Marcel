# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua5.3 ) -DLUA -DDEBUG -DMCHECK="mcheck(NULL)"

mod_freebox.o : mod_freebox.c mod_freebox.h ../Module.h ../Section.h \
  ../MQTT_tools.h Makefile 
	$(cc) -c -o mod_freebox.o mod_freebox.c $(opts) 

../../mod_freebox.so : mod_freebox.o Makefile 
	 $(cc) -shared -o ../../mod_freebox.so mod_freebox.o $(opts) \
  

all: ../../mod_freebox.so 
