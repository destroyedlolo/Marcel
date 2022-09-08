# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=gcc -Wall -O2 -DDEBUG
opts=

DList.o : DList.c DList.h Makefile 
	$(cc) -c -o DList.o DList.c $(opts) 

Marcel.o : Marcel.c Marcel.h Version.h Makefile 
	$(cc) -c -o Marcel.o Marcel.c $(opts) 

module.o : module.c module.h Makefile 
	$(cc) -c -o module.o module.c $(opts) 

../Marcel : module.o Marcel.o DList.o Makefile 
	 $(cc) -o ../Marcel module.o Marcel.o DList.o $(opts) 

all: ../Marcel 
