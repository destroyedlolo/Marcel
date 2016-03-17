#!/bin/bash

#this script is used to create a Makefile

cd src

LFMakeMaker -v +f=Makefile -cc='gcc -Wall -O2 -DFREEBOX -DUPS -DMETEO -lcurl -lpthread -lpaho-mqtt3c -DLUA `pkg-config --cflags lua` `pkg-config --libs lua` `pkg-config --cflags json-c` `pkg-config --libs json-c` -std=c99' *.c -t=../Marcel > Makefile
