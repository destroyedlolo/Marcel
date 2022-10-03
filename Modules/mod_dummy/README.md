# Mod_Dummy

It's a "*fake*" module only shows how to create a module for Marcel.

Its source code is heavy and extensively commented to guide you in the process of building a module.
It's featuring :
* top level configuration option
* section definition and code implementation
* Lua callback (Lua is enabled, obviously)

## installation 
* if you're building from source, in **remake.sh** enable `BUILD_DUMMY`
* in **<config_dir>/00_Marcel**, enable `LoadModule=mod_dummy.so`
* copy **config/50_Dummy** in **<config_dir>/**
* copy **config/scripts/Dummy_Sample.lua** in **<config_dir>/scripts**
