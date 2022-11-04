# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua ) -DLUA -DDEBUG -DMCHECK="mcheck(NULL)"

mod_OnOff.o : mod_OnOff.c Makefile 
	$(cc) -c -o mod_OnOff.o mod_OnOff.c $(opts) 

../../mod_OnOff.so : mod_OnOff.o Makefile 
	 $(cc) -shared -o ../../mod_OnOff.so mod_OnOff.o $(opts) 

all: ../../mod_OnOff.so 
