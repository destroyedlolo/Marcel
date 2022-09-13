# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC -DDEBUG

mod_test.o : mod_test.c mod_test.h ../Module.h Makefile 
	$(cc) -c -o mod_test.o mod_test.c $(opts) 

../../mod_test.so : mod_test.o Makefile 
	 $(cc) -shared -o ../../mod_test.so mod_test.o $(opts) 

all: ../../mod_test.so 
