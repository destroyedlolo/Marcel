# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.3 (Apr 13 2016 21:34:06) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=gcc -Wall -O2 -DRFXTRX -DFREEBOX -DUPS -DMETEO -lcurl -lpthread -lpaho-mqtt3c -DLUA `pkg-config --cflags lua` `pkg-config --libs lua` `pkg-config --cflags json-c` `pkg-config --libs json-c` -std=c99

Alerting.o : Alerting.c Marcel.h Alerting.h DList.h Makefile 
	$(cc) -c -o Alerting.o Alerting.c 

DeadPublisherDetection.o : DeadPublisherDetection.c \
  DeadPublisherDetection.h Marcel.h MQTT_tools.h Makefile 
	$(cc) -c -o DeadPublisherDetection.o DeadPublisherDetection.c 

DList.o : DList.c DList.h Makefile 
	$(cc) -c -o DList.o DList.c 

Every.o : Every.c Every.h Marcel.h Makefile 
	$(cc) -c -o Every.o Every.c 

Freebox.o : Freebox.c Freebox.h Marcel.h MQTT_tools.h Makefile 
	$(cc) -c -o Freebox.o Freebox.c 

Lua.o : Lua.c Marcel.h Alerting.h DList.h MQTT_tools.h Makefile 
	$(cc) -c -o Lua.o Lua.c 

Marcel.o : Marcel.c Marcel.h Freebox.h UPS.h DeadPublisherDetection.h \
  MQTT_tools.h Alerting.h DList.h Every.h Meteo.h RFXtrx_marcel.h \
  Makefile 
	$(cc) -c -o Marcel.o Marcel.c 

Meteo.o : Meteo.c Meteo.h Marcel.h MQTT_tools.h Makefile 
	$(cc) -c -o Meteo.o Meteo.c 

MQTT_tools.o : MQTT_tools.c MQTT_tools.h Makefile 
	$(cc) -c -o MQTT_tools.o MQTT_tools.c 

RFXtrx_marcel.o : RFXtrx_marcel.c RFXtrx_marcel.h Marcel.h Makefile 
	$(cc) -c -o RFXtrx_marcel.o RFXtrx_marcel.c 

UPS.o : UPS.c UPS.h Marcel.h MQTT_tools.h Makefile 
	$(cc) -c -o UPS.o UPS.c 

../Marcel : UPS.o RFXtrx_marcel.o MQTT_tools.o Meteo.o Marcel.o Lua.o \
  Freebox.o Every.o DList.o DeadPublisherDetection.o Alerting.o \
  Makefile 
	 $(cc) -o ../Marcel UPS.o RFXtrx_marcel.o MQTT_tools.o Meteo.o \
  Marcel.o Lua.o Freebox.o Every.o DList.o DeadPublisherDetection.o \
  Alerting.o 

all: ../Marcel 
