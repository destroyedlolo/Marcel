#!/bin/bash

# Target directories

BIN=/usr/local/sbin
LIB=/usr/local/lib/Marcel

# No user changes expected bellow

mkdir -p ${LIB} || { echo "can't create target directory, please check if your user has enough rights" ; exit 0; }

cp *.so ${LIB}

cp Marcel ${BIN}

echo "please run ldconfig as root"
