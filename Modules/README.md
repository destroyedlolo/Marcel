# modules documentation

**Marcel** is composed of many modules to implement new interfaces and features. 
It's wise to avoid intermodule dependancies with the exception of **mod_core** components and **mod_lua**.
Each of the modules are documented in their own directory.

## configuration

Line starting by an hash (#) are ignored as considered as comment.<br>
Notez-bien : '#' and top level directive must start at the 1st character.

### Variables substitution

Substitution is done globally in configuration files.

* **%ClientID%** is replaced by the MQTT's ClientID. 
* **%Hostname%** is replaced by the hostname

### Enabling modules

Copy and customize from module's `Config` subdirectory to your main configuration directory. Then restart **Marcel**

### Debuging your configuration

The configuration can be tested using `-t` flag : Marcel will ensure the syntax correctness and exits.

`-v` flag will make Marcel verbose : display how it understands the configuration, and displays some runtime figures.

`-d` (if Marcel has been compiled for), will display debuging information, mostly technical runtime information.

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
