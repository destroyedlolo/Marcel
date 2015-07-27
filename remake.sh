#!/bin/bash

#this script is used to create a Makefile

LFMakeMaker -v +g -Isrc/ -cc='gcc -Wall -DFREEBOX -DUPS -lcurl -lpthread -lpaho-mqtt3c -DLUA -llua -std=c99' src/*.c -t=Marcel > Makefile
