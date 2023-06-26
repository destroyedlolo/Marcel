Build from source
====

In case you want to install **Marcel** from scratch or if it hasn't been packaged for your distribution.

# Requirements :

## Compilation environment

* a **C compile chaine** : Gcc, Make, ...
On Debian derived, you need something like
```
apt install git make build-essential
```
Have a look on your distribution documentation :smirk:

* If you need to modify some compilation options ; [LFMakeMaker](http://destroyedlolo.info/Developpement/LFMakeMaker/).<br>
I strongly suggest to modify then launch **remake.sh** instead of tedious Makefile changes.

## Helpers libraries

### Mandatory library
* [Paho](https://eclipse.org/paho/clients/c/) as MQTT communication layer.

### Modules dependant optional libraries
* [json-c](https://github.com/json-c/json-c/wiki)
(debian package : `libjson-c-dev`)

* [libcurl](https://curl.se/libcurl/)

# Compilation :
## get sources
Get **Marcel** latest tarball for *stable* version or clone its repository for rolling release version
(if you get the default branch, you're guaranteed to get a stable and tested version).* Install **PAHO** library for C ( https://eclipse.org/paho/clients/c/ )
```
git clone https://github.com/destroyedlolo/Marcel.git
```
## Select which modules to build

The default Makefile will build all modules. If you're having the need to exclude a module from building
(for exemple, in case of technical dependancy issue), you have to :
* Enable/Disable corresponding **BUILD_????** option in `remake.sh` shell script.
* Run `remake.sh` to update Makefiles

**Notez-bien :** we are speaking here of **compiling**. If your need is only to avoid a module to run, it can be done directly from the configuration files, no need to recompile anything.

## Compile

From the top directory
```
make
```

## install

As root, 
```
install.sh
```
You may first having a look in this script if you don't want to install Marcel in the default location.

