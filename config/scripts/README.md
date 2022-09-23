This directory contains Lua scripts that are defining user's functions.

Notez-bien : all codes share the same Lua's state. So all identifier **must be unique**.

The following global variables are defined :
- **MARCEL_SCRIPT_DIR** : where are stored Lua scripts
- **MARCEL_DEBUG** : only defined if Marcel is running in debug mode (intended to add debugging verbosity)

The following functions are exposed :
- **Marcel.Hostname()** : returns the host's name
- **Marcel.ClientID()** : returns MQTT client ID as defined in a configuration file or automatically generated
- **Marcel.Version()** : returns Marcel's version
- **Marcel.Copyright()** : returns Marcel's configuration string.
