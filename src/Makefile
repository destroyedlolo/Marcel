# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  9 2022 15:11:43) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=gcc -Wall -O2 -DDEBUG
opts=

DList.o : DList.c DList.h Makefile 
	$(cc) -c -o DList.o DList.c $(opts) 

Marcel.o : Marcel.c Marcel.h Version.h module_core.h module.h Makefile 
	$(cc) -c -o Marcel.o Marcel.c $(opts) 

module.o : module.c module.h Makefile 
	$(cc) -c -o module.o module.c $(opts) 

module_core.o : module_core.c module_core.h module.h Makefile 
	$(cc) -c -o module_core.o module_core.c $(opts) 

../Marcel : module_core.o module.o Marcel.o DList.o Makefile 
	 $(cc) -o ../Marcel module_core.o module.o Marcel.o DList.o \
  $(opts) 

all: ../Marcel 
