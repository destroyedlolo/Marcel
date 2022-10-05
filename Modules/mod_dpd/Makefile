# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua5.3 ) -DLUA -DDEBUG -DMCHECK="mcheck(NULL)"

mod_dpd.o : mod_dpd.c mod_dpd.h ../Marcel/Module.h ../Marcel/Section.h \
  ../mod_Lua/mod_Lua.h Makefile 
	$(cc) -c -o mod_dpd.o mod_dpd.c $(opts) 

../../mod_dpd.so : mod_dpd.o Makefile 
	 $(cc) -shared -o ../../mod_dpd.so mod_dpd.o $(opts) 

all: ../../mod_dpd.so 
