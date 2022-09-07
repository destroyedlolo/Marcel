#!/bin/bash

#this script is used to create a Makefile

cd src

LFMakeMaker -v +f=Makefile -cc='gcc -Wall -O2 -DDEBUG' *.c -t=../Marcel > Makefile
