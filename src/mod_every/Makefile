# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  9 2022 15:11:43) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua5.4 ) -DLUA -DDEBUG -DMCHECK="mcheck(NULL)"

mod_every.o : mod_every.c mod_every.h ../Module.h ../Section.h \
  ../mod_Lua/mod_Lua.h Makefile 
	$(cc) -c -o mod_every.o mod_every.c $(opts) 

../../mod_every.so : mod_every.o Makefile 
	 $(cc) -shared -o ../../mod_every.so mod_every.o $(opts) 

all: ../../mod_every.so 
