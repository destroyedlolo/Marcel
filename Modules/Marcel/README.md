Marcel's core and utilities
======

This directory contains Marcel's core and shared mandatory utilities codes.

### Global directives

* **ClientID=** ClientID to connect to the broker. 
In case you have more than one Marcel connected to a single broker you MUST set a unique ID per Marcel instance.<br>
If not set, a random identifier based on the host name and the process ID is used.

* **Broker=** MQTT Broker's URL [Mandatory]<br>
Example : 
`Broker=tcp://localhost:1883`

* **LoadModule=** Load a module.<br>
Example : loading Lua module.<br>
`LoadModule=mod_Lua.so`

* **Include=** Load configuration files from provided directory.<br>

* **SubLast** indicates that all sections doing MQTT subscription
are grouped at the end of the configuration.<br>
It's an optimization option : Marcel will scan subscriptions from
the end of the configuration and will stop as soon as it found a section doesn't need an MQTT subscription.

* **Simulation** Run in simulation mode

### Directive understood by almost all configuration files

* **Needs=** Ignore this configuration file is a module is not loaded.<br>
Example : only consider this configuration if `mod_dummy` is loaded<br>
`Needs=mod_dummy`
