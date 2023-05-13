# modules documentation

**Marcel** is composed of many modules to implement new interfaces and features. 
It's wise to avoid intermodule dependancies with the exception of **mod_core** components and **mod_lua**.
Each of the modules are documented in their own directory.

## configuration

### Enabling modules

Copy and customize from module's `Config` subdirectory to your main configuration directory. Then restart **Marcel**

### Debuging your configuration

The configuration can be tested using `-t` flag : Marcel will ensure the syntax correctness and exits.

`-v` flag will make Marcel verbose : display how it understands the configuration, and displays some runtime figures.

`-d` (if Marcel has been compiled for), will display debuging information, mostly technical runtime information.

## Building from source

Please have a look on the build instruction for Marcel itself (in the parent directory).

### Select which modules to build

The default Makefile will build all modules. If you're having the need to exclude a module from building (for exemple, in case of technical dependancy issue), you have to :
* Enable/Disable corresponding **BUILD_????** option in `remake.sh` shell script.
* Run `remake.sh` to update Makefiles
* `make` to compile the code.
* install executable and `.so`.

**Notez-bien :** we are speaking here of **compiling**. If your need is only to avoid a module to run, it can be done directly from the configuration files, no need to recompile anything.

## specific modules

Modules' specific information can be found in their own directories.

Among modules, following ones have some specificities.

### Marcel

This directory contains mandatory core features and modules. Consequently, it can't be disabled.

### mod_lua

This module provides **Lua** support to other modules. If enabled, it enable user functions and scripts.

### mod_Dummy

This module is providing as an example "*how to create your own module*". Its source code is extensively commented.

As it doesn't provide anything useful, it is not expected to be enabled on a production environment.
