# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (May  1 2018 12:31:45) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=gcc -Wall -O2 -DxDEBUG -DRFXTRX -DFREEBOX -DUPS -DMETEO -DINOTIFY -DSHT31 -lcurl -lpthread -lpaho-mqtt3c -DLUA `pkg-config --cflags lua` `pkg-config --libs lua` `pkg-config --cflags json-c` `pkg-config --libs json-c` -std=c99
opts=

Alerting.o : Alerting.c Marcel.h Alerting.h DList.h MQTT_tools.h \
  Makefile 
	$(cc) -c -o Alerting.o Alerting.c $(opts) 

CURL_helpers.o : CURL_helpers.c CURL_helpers.h Marcel.h Makefile 
	$(cc) -c -o CURL_helpers.o CURL_helpers.c $(opts) 

DeadPublisherDetection.o : DeadPublisherDetection.c \
  DeadPublisherDetection.h Marcel.h MQTT_tools.h Makefile 
	$(cc) -c -o DeadPublisherDetection.o DeadPublisherDetection.c \
  $(opts) 

DList.o : DList.c DList.h Makefile 
	$(cc) -c -o DList.o DList.c $(opts) 

Every.o : Every.c Every.h REST.h Marcel.h Makefile 
	$(cc) -c -o Every.o Every.c $(opts) 

FFV.o : FFV.c Marcel.h FFV.h MQTT_tools.h Makefile 
	$(cc) -c -o FFV.o FFV.c $(opts) 

Freebox.o : Freebox.c Freebox.h Marcel.h MQTT_tools.h Makefile 
	$(cc) -c -o Freebox.o Freebox.c $(opts) 

Lua.o : Lua.c Marcel.h Version.h Alerting.h DList.h MQTT_tools.h \
  Makefile 
	$(cc) -c -o Lua.o Lua.c $(opts) 

Marcel.o : Marcel.c Marcel.h Version.h FFV.h Freebox.h UPS.h \
  DeadPublisherDetection.h MQTT_tools.h Alerting.h DList.h Every.h \
  Meteo.h RFXtrx_marcel.h REST.h OutFile.h Sht31.h Makefile 
	$(cc) -c -o Marcel.o Marcel.c $(opts) 

Meteo.o : Meteo.c Meteo.h Marcel.h Version.h MQTT_tools.h \
  CURL_helpers.h Makefile 
	$(cc) -c -o Meteo.o Meteo.c $(opts) 

MQTT_tools.o : MQTT_tools.c MQTT_tools.h Makefile 
	$(cc) -c -o MQTT_tools.o MQTT_tools.c $(opts) 

OutFile.o : OutFile.c OutFile.h Marcel.h Makefile 
	$(cc) -c -o OutFile.o OutFile.c $(opts) 

REST.o : REST.c REST.h Marcel.h Version.h CURL_helpers.h Makefile 
	$(cc) -c -o REST.o REST.c $(opts) 

RFXtrx_marcel.o : RFXtrx_marcel.c RFXtrx_marcel.h Marcel.h RFXtrx.h \
  Makefile 
	$(cc) -c -o RFXtrx_marcel.o RFXtrx_marcel.c $(opts) 

Sht31.o : Sht31.c Marcel.h Sht31.h MQTT_tools.h Makefile 
	$(cc) -c -o Sht31.o Sht31.c $(opts) 

UPS.o : UPS.c UPS.h Marcel.h MQTT_tools.h Makefile 
	$(cc) -c -o UPS.o UPS.c $(opts) 

../Marcel : UPS.o Sht31.o RFXtrx_marcel.o REST.o OutFile.o \
  MQTT_tools.o Meteo.o Marcel.o Lua.o Freebox.o FFV.o Every.o DList.o \
  DeadPublisherDetection.o CURL_helpers.o Alerting.o Makefile 
	 $(cc) -o ../Marcel UPS.o Sht31.o RFXtrx_marcel.o REST.o \
  OutFile.o MQTT_tools.o Meteo.o Marcel.o Lua.o Freebox.o FFV.o \
  Every.o DList.o DeadPublisherDetection.o CURL_helpers.o Alerting.o \
  $(opts) 

all: ../Marcel 
