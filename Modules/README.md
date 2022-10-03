# Source code and modules documentation

**Marcel** is composed of many modules that implements new interfaces and features. 
They generally don't dependent on others but *core* components and **mod_lua**.
Each of the modules are documented in their own directorie.

## modules

Here are the steps to add a module to Marcel.

### compilation (only needed if Marcel is not packaged for your distribution)

* Enable corresponding **BUILD_????** option in `remake.sh` shell script.
* Run `remake.sh` to update Makefiles
* `make` to compile the code.
* install executable and `.so`.

### Enable the module

Copy and customize files in module's `Config` subdirectory to your configuration directory. Then restart **Marcel**

## specific modules

### Marcel

This directory contains mandatory core features and modules. Consequently, it can't be disabled.

### mod_lua

This module provides **Lua** support to other modules and, consequently, enable user functions.

### mod_Dummy

This module is providing as an example "*how to create your own module*". Its source code is extensively commented.

As it doesn't provide anything useful, it is not expected to be enabled on a production environment.
