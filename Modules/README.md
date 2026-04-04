# modules documentation

**Marcel** is built on a modular architecture to facilitate the integration of new interfaces and features.  
Each of the modules is documented in its own directory.

## configuration

Lines starting with a hash (#) are ignored, as considered comments.<br>
Notez-bien : '#' and top level directives must start at the 1st character.

### Variables substitution

Global substitution is applied across all configuration files:

* **%ClientID%** is replaced by the MQTT's ClientID. 
* **%Hostname%** is replaced by the hostname.

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

> [!CAUTION]
> If you're creating a module, tt's wise to avoid inter-module dependencies, except **mod_core** components and **mod_lua**.

When a dependency between modules is strictly necessary, use the `findModuleByName()` function to retrieve the
module structure and access its callbacks. Always verify the return value of `findModuleByName();` if it returns `NULL`,
the module is not loaded and cannot be used.
