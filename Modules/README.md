# modules documentation

**Marcel** is composed of many modules to implement new interfaces and features. 
It's wise to avoid inter-module dependencies, except **mod_core** components and **mod_lua**.
Each of the modules is documented in its own directory.

In cases where inter-module dependence is mandatory, use `findModuleByName()` function to find out the module structure, to get access to some callbacks. Never forget **to check `findModuleByName()` returns**. If NULL, it's meaning the module is not loaded and so can't be used.


## configuration

Lines starting with a hash (#) are ignored, as considered comments.<br>
Notez-bien : '#' and top level directives must start at the 1st character.

### Variables substitution

Substitution is done globally in configuration files.

* **%ClientID%** is replaced by the MQTT's ClientID. 
* **%Hostname%** is replaced by the hostname

### Enabling modules

Copy and customize from the module's `Config` subdirectory to your main configuration directory. Then restart **Marcel**

### Debugging your configuration

The configuration can be tested using the `-t` flag : Marcel will ensure the syntax is correct and exits.

`-v` flag will make Marcel verbose : display how it understands the configuration, and display some runtime figures.

`-d` (if Marcel has been compiled for) will display debugging information, mostly technical runtime information.

## specific modules

Modules' specific information can be found in their own directories.

Among the modules, the following ones have some specificities.

### Marcel

This directory contains mandatory core features and modules. Consequently, it can't be disabled.

### mod_lua

This module provides **Lua** support for other modules. If enabled, it enables user functions and scripts.

### mod_Dummy

This module is providing an example of "*how to create your own module*". Its source code is extensively commented.

As it doesn't provide anything useful, it is not expected to be enabled in a production environment.
