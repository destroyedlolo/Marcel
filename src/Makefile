# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.3 (Apr 13 2016 21:34:06) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=gcc -Wall -O2 -DRFXTRX -DFREEBOX -DUPS -DMETEO -lcurl -lpthread -lpaho-mqtt3c -DLUA `pkg-config --cflags lua` `pkg-config --libs lua` `pkg-config --cflags json-c` `pkg-config --libs json-c` -std=c99

Alerting.o : Alerting.c Marcel.h Alerting.h DList.h Makefile 
	$(cc) -c -o Alerting.o Alerting.c 

CURL_helpers.o : CURL_helpers.c CURL_helpers.h Makefile 
	$(cc) -c -o CURL_helpers.o CURL_helpers.c 

DList.o : DList.c DList.h Makefile 
	$(cc) -c -o DList.o DList.c 

DeadPublisherDetection.o : DeadPublisherDetection.c \
  DeadPublisherDetection.h Marcel.h MQTT_tools.h Makefile 
	$(cc) -c -o DeadPublisherDetection.o DeadPublisherDetection.c 

Every.o : Every.c Every.h Marcel.h Makefile 
	$(cc) -c -o Every.o Every.c 

FFV.o : FFV.c Marcel.h FFV.h MQTT_tools.h Makefile 
	$(cc) -c -o FFV.o FFV.c 

Freebox.o : Freebox.c Freebox.h Marcel.h MQTT_tools.h Makefile 
	$(cc) -c -o Freebox.o Freebox.c 

Lua.o : Lua.c Marcel.h Version.h Alerting.h DList.h MQTT_tools.h \
  Makefile 
	$(cc) -c -o Lua.o Lua.c 

MQTT_tools.o : MQTT_tools.c MQTT_tools.h Makefile 
	$(cc) -c -o MQTT_tools.o MQTT_tools.c 

Marcel.o : Marcel.c Marcel.h Version.h FFV.h Freebox.h UPS.h \
  DeadPublisherDetection.h MQTT_tools.h Alerting.h DList.h Every.h \
  Meteo.h RFXtrx_marcel.h REST.h Makefile 
	$(cc) -c -o Marcel.o Marcel.c 

Meteo.o : Meteo.c Meteo.h Marcel.h Version.h MQTT_tools.h \
  CURL_helpers.h Makefile 
	$(cc) -c -o Meteo.o Meteo.c 

REST.o : REST.c Every.h Version.h Marcel.h CURL_helpers.h Makefile 
	$(cc) -c -o REST.o REST.c 

RFXtrx_marcel.o : RFXtrx_marcel.c RFXtrx_marcel.h Marcel.h RFXtrx.h \
  Makefile 
	$(cc) -c -o RFXtrx_marcel.o RFXtrx_marcel.c 

UPS.o : UPS.c UPS.h Marcel.h MQTT_tools.h Makefile 
	$(cc) -c -o UPS.o UPS.c 

../Marcel : UPS.o RFXtrx_marcel.o REST.o Meteo.o Marcel.o MQTT_tools.o \
  Lua.o Freebox.o FFV.o Every.o DeadPublisherDetection.o DList.o \
  CURL_helpers.o Alerting.o Makefile 
	 $(cc) -o ../Marcel UPS.o RFXtrx_marcel.o REST.o Meteo.o \
  Marcel.o MQTT_tools.o Lua.o Freebox.o FFV.o Every.o \
  DeadPublisherDetection.o DList.o CURL_helpers.o Alerting.o 

all: ../Marcel 
