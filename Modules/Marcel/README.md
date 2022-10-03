# Marcel's core and utilities

This directory contains Marcel's core and shared mandatory utilities codes.

### Global directives

* **ClientID=** ClientID to connect to the broker. 
In case you have more than one Marcel connected to a single broker you MUST set an uniq ID per Marcel instance.<br>
[Optional] default : random identifier based on the host name and the process ID.

* **Broker=** MQTT Broker's URL [Mandatory]<br>
Example : 
`Broker=tcp://localhost:1883`

* **LoadModule=** Load a Marcel' module.<br>
Example : loading Lua module.<br>
`LoadModule=mod_Lua.so`

### Directive understood by very configuration file

* **Needs=** Ignore this configuration file is a module is not loaded.<br>
Example : only took in account if `mod_dummy` is loaded<br>
`Needs=mod_dummy`
