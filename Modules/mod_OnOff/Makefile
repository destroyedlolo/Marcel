# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua5.3 ) -DLUA  

mod_OnOff.o : mod_OnOff.c mod_OnOff.h ../Marcel/Module.h \
  ../Marcel/Section.h Makefile 
	$(cc) -c -o mod_OnOff.o mod_OnOff.c $(opts) 

../../mod_OnOff.so : mod_OnOff.o Makefile 
	 $(cc) -shared -o ../../mod_OnOff.so mod_OnOff.o $(opts) 

all: ../../mod_OnOff.so 
