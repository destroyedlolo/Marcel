# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua ) -DLUA  

mod_freebox.o : mod_freebox.c mod_freebox.h ../Marcel/Module.h \
  ../Marcel/Section.h ../Marcel/MQTT_tools.h Makefile 
	$(cc) -c -o mod_freebox.o mod_freebox.c $(opts) 

../../mod_freebox.so : mod_freebox.o Makefile 
	 $(cc) -shared -o ../../mod_freebox.so mod_freebox.o $(opts) \
  

all: ../../mod_freebox.so 
