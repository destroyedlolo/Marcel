# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.6 (May  7 2022 20:46:23) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=-Wall -O2 -fPIC $(shell pkg-config --cflags lua5.3 ) -DLUA  

action.o : action.c mod_alert.h ../Marcel/Module.h ../Marcel/Section.h \
  ../Marcel/DList.h ../mod_Lua/mod_Lua.h Makefile 
	$(cc) -c -o action.o action.c $(opts) 

alert.o : alert.c mod_alert.h ../Marcel/Module.h ../Marcel/Section.h \
  ../Marcel/DList.h ../mod_Lua/mod_Lua.h ../Marcel/MQTT_tools.h \
  Makefile 
	$(cc) -c -o alert.o alert.c $(opts) 

mod_alert.o : mod_alert.c mod_alert.h ../Marcel/Module.h \
  ../Marcel/Section.h ../Marcel/DList.h ../mod_Lua/mod_Lua.h Makefile 
	$(cc) -c -o mod_alert.o mod_alert.c $(opts) 

notification.o : notification.c mod_alert.h ../Marcel/Module.h \
  ../Marcel/Section.h ../Marcel/DList.h ../mod_Lua/mod_Lua.h \
  ../Marcel/MQTT_tools.h Makefile 
	$(cc) -c -o notification.o notification.c $(opts) 

../../mod_alert.so : notification.o mod_alert.o alert.o action.o \
  Makefile 
	 $(cc) -shared -o ../../mod_alert.so notification.o \
  mod_alert.o alert.o action.o $(opts) 

all: ../../mod_alert.so 
