# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC -DDEBUG 	-DPLUGIN_DIR='"/home/laurent/Projets/Marcel"' -L/home/laurent/Projets/Marcel 	-L/home/laurent/Projets/Marcel -lpaho-mqtt3c  -lm -ldl -Wl,--export-dynamic -lpthread 	

Marcel.o : Marcel.c Marcel.h Version.h mod_core.h Module.h Section.h \
  Makefile 
	$(cc) -c -o Marcel.o Marcel.c $(opts) 

mod_core.o : mod_core.c mod_core.h Module.h Marcel.h Section.h \
  Makefile 
	$(cc) -c -o mod_core.o mod_core.c $(opts) 

Module.o : Module.c Module.h Marcel.h Section.h Makefile 
	$(cc) -c -o Module.o Module.c $(opts) 

../Marcel : Module.o mod_core.o Marcel.o Makefile 
	 $(cc) -o ../Marcel Module.o mod_core.o Marcel.o $(opts) 

all: ../Marcel 
