# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua ) -DLUA -DDEBUG -DMCHECK="mcheck(NULL)"

mod_alert.o : mod_alert.c mod_alert.h ../Marcel/Module.h \
  ../Marcel/Section.h Makefile 
	$(cc) -c -o mod_alert.o mod_alert.c $(opts) 

notification.o : notification.c mod_alert.h ../Marcel/Module.h \
  ../Marcel/Section.h ../Marcel/MQTT_tools.h Makefile 
	$(cc) -c -o notification.o notification.c $(opts) 

../../mod_alert.so : notification.o mod_alert.o Makefile 
	 $(cc) -shared -o ../../mod_alert.so notification.o \
  mod_alert.o $(opts) 

all: ../../mod_alert.so 
