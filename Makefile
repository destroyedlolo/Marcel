# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.2 (May 30 2015 17:53:45) (c)LFSoft 1997

gotoall: all

# Warning : 'sys/time.h' can't be located for this node.
# Warning : 'sys/select.h' can't be located for this node.
# Warning : 'sys/eventfd.h' can't be located for this node.
# Warning : 'unistd.h' can't be located for this node.

#The compiler (may be customized for compiler's options).
cc=gcc -Wall -DFREEBOX -DUPS -lpthread -lpaho-mqtt3c -std=c99

src/DeadPublisherDetection.o : src/DeadPublisherDetection.c \
  src/DeadPublisherDetection.h 
	$(cc) -c -o src/DeadPublisherDetection.o \
  src/DeadPublisherDetection.c 

# Warning : 'string.h' can't be located for this node.
# Warning : 'stdio.h' can't be located for this node.
# Warning : 'assert.h' can't be located for this node.
# Warning : 'errno.h' can't be located for this node.
# Warning : 'unistd.h' can't be located for this node.
# Warning : 'sys/types.h' can't be located for this node.
# Warning : 'sys/socket.h' can't be located for this node.
# Warning : 'netinet/in.h' can't be located for this node.
# Warning : 'netdb.h' can't be located for this node.
src/Freebox.o : src/Freebox.c src/MQTT_tools.h src/Freebox.h 
	$(cc) -c -o src/Freebox.o src/Freebox.c 

# Warning : 'stdio.h' can't be located for this node.
# Warning : 'stdlib.h' can't be located for this node.
# Warning : 'string.h' can't be located for this node.
# Warning : 'strings.h' can't be located for this node.
# Warning : 'errno.h' can't be located for this node.
# Warning : 'libgen.h' can't be located for this node.
# Warning : 'assert.h' can't be located for this node.
# Warning : 'unistd.h' can't be located for this node.
# Warning : 'signal.h' can't be located for this node.
# Warning : 'sys/types.h' can't be located for this node.
# Warning : 'sys/socket.h' can't be located for this node.
# Warning : 'netinet/in.h' can't be located for this node.
# Warning : 'netdb.h' can't be located for this node.
src/Marcel.o : src/Marcel.c src/MQTT_tools.h \
  src/DeadPublisherDetection.h src/UPS.h src/Freebox.h src/Marcel.h 
	$(cc) -c -o src/Marcel.o src/Marcel.c 

src/MQTT_tools.o : src/MQTT_tools.c src/MQTT_tools.h 
	$(cc) -c -o src/MQTT_tools.o src/MQTT_tools.c 

# Warning : 'string.h' can't be located for this node.
# Warning : 'errno.h' can't be located for this node.
# Warning : 'assert.h' can't be located for this node.
# Warning : 'unistd.h' can't be located for this node.
# Warning : 'sys/types.h' can't be located for this node.
# Warning : 'sys/socket.h' can't be located for this node.
# Warning : 'netinet/in.h' can't be located for this node.
# Warning : 'netdb.h' can't be located for this node.
src/UPS.o : src/UPS.c src/MQTT_tools.h src/UPS.h 
	$(cc) -c -o src/UPS.o src/UPS.c 

Marcel : src/UPS.o src/MQTT_tools.o src/Marcel.o src/Freebox.o \
  src/DeadPublisherDetection.o 
	 $(cc) -o Marcel src/UPS.o src/MQTT_tools.o src/Marcel.o \
  src/Freebox.o src/DeadPublisherDetection.o 

all: Marcel 
