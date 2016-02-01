# Marcel
**Marcel** is a lightweight versatile daemon to
- publish easily some figures to an **MQTT broker** (1wire probe values, UPS, ...)
- watchdog on published data 
- custom checks can be implemented through Lua scripts
- finally send SMS in case a test failed and then when it recovers.

#Requirements :#
* MQTT broker (I personally use [Mosquitto](http://mosquitto.org/) )
* [Paho](http://eclipse.org/paho/) as MQTT communication layer.

#Compilation :#
* Get **Marcel** latest tarball for *stable* version or clone its repository for *development* version
* Install **PAHO** library for C ( https://eclipse.org/paho/clients/c/ )
* **curl** is needed as well to send SMS
* Compile Marcel

    make

You may modify the **Makefile** to add or remove following compiler options :
* **-DFREEBOX** : Enable [Freebox](https://en.wikipedia.org/wiki/Freebox) figures publishing (French **I**nternet **S**ervice **P**rovider)
* **-DUPS** : Enable UPS monitoring. Query [NUT](www.networkupstools.org/) through its Telnet interface to retrieve UPS figures.
* **-DLUA** : Enable Lua user function (v3.0+)

##Note :##
This Makefile is automatically generated by my old (but still useful :) ) [LFMakeMaker](http://destroyedlolo.info/Developpement/LFMakeMaker/).
I strongly suggest to modify then launch **remake.sh** instead of tedious Makefile changes.

#Configuration :#
By default, Marcel reads **/usr/local/etc/Marcel.conf** as configuration file (may be changed using command line option **-f**).
Have a look on provided file which contains comprehensive explanation of known directives.

#Lua custom methods :#
Following methods are exposed to Lua code through **Marcel** object :
* **Marcel.MQTTPublish(** topic, value **)** : Publish a value to MQTT broker

* **Marcel.RiseAlert(** topic**,** message **)** : Tells Marcel about an alert condition
* **Marcel.ClearAlert(** topic **)** : Clear an alert condition

* **Marcel.SendMessage(** title **,** Text **)** : send a message using AlertCommand facility. Generally used to send a mail (which is not considered as an alert)

* **Marcel.Hostname()** : As the name said, host's name
* **Marcel.ClientID()** : Configured MQTT client id
* **Marcel.Version()** : Marcel's version (*have a look on **scripts/AllVerif.lua** for an usage example*)

**Notez-bien :** Marcel is able run only **one Lua function at once**. Consequently, your functions have to be as fast as possible.

#Installation :#
A startup script for OpenRC has been provided in sub-directory ... startup_scripts.

#Launch options :#
Marcel knows the following options :
* *-v* : verbose output
* *-f<file>* : loads <file> as configuration file. The default one is `/usr/local/etc/Marcel.conf`

Have a look on provided configuration file to guess the syntax used (I'm busy, a full documentation will come later).

#Side note#
The name is a tribute to my late rabbit that passed away some days before I did started this project : he stayed at home as keeper. RIP.
> Written with [StackEdit](https://stackedit.io/).
