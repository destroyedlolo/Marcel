# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua ) -DLUA -DDEBUG -DMCHECK="mcheck(NULL)"

mod_owm.o : mod_owm.c mod_owm.h ../Marcel/Module.h ../Marcel/Section.h \
  Makefile 
	$(cc) -c -o mod_owm.o mod_owm.c $(opts) 

../../mod_owm.so : mod_owm.o Makefile 
	 $(cc) -shared -o ../../mod_owm.so mod_owm.o $(opts) 

all: ../../mod_owm.so 
