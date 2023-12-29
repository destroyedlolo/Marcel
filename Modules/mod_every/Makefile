# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua ) -DLUA  

mod_every.o : mod_every.c mod_every.h ../Marcel/Module.h \
  ../Marcel/Section.h ../mod_Lua/mod_Lua.h Makefile 
	$(cc) -c -o mod_every.o mod_every.c $(opts) 

../../mod_every.so : mod_every.o Makefile 
	 $(cc) -shared -o ../../mod_every.so mod_every.o $(opts) 

all: ../../mod_every.so 
