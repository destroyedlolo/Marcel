# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua ) -DLUA -DDEBUG -DMCHECK="mcheck(NULL)"

mod_sht31.o : mod_sht31.c mod_sht31.h ../Marcel/Module.h \
  ../Marcel/Section.h ../mod_Lua/mod_Lua.h Makefile 
	$(cc) -c -o mod_sht31.o mod_sht31.c $(opts) 

../../mod_sht31.so : mod_sht31.o Makefile 
	 $(cc) -shared -o ../../mod_sht31.so mod_sht31.o $(opts) 

all: ../../mod_sht31.so 
