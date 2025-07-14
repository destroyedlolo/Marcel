Marcel
===
**Marcel** is a lightweight, versatile **MQTT data publisher**.

Focusing on smart home automation (but not only), its optional modules publish : 
- 1-wire probes environmental figures
- weather forecast
- UPS figures
- external MQTT events
- and many more

Can raise some notifications, manage alerts, and control some devices. 

**Lua user scripts** can be used to build simple automation and/or, to validate incoming values.

Thanks to its open and powerful module's API, it's *easy* to add new functionalities.

`Build from source.md` contains technical information if you want to compile directly from its source code.

# Dependencies

As the communication is based on MQTT messages, you obviously need a ... **broker** :
I personally use [Mosquitto](http://mosquitto.org/).

## Global dependency

Install **PAHO** library for C ( https://eclipse.org/paho/clients/c/ )

## Modules related runtime dependencies

### Lua (mod_Lua)
If you want to have *user*'s functions : https://www.lua.org/

### 1-wire (mod_1wire)
Recent Linux kernel has *better than nothing* and *limited* 1-wire support. I strongly suggest using [OWFS](https://www.owfs.org/) instead.

### OpenWeatherMap (mod_OpenWeatherMap)
You need to provide your own license key to query the weather forecast, it's free for hobbyist usage.<br>
Take a look on : https://openweathermap.org/

In addition, [json-c](https://github.com/json-c/json-c/wiki) and [libcurl](https://curl.se/libcurl/) are needed.

## Launch options :
Marcel knows the following options :
* *-h* : online help
* *-v* : verbose output
* *-S* : runs in "Simulation" mode
* *-f<directory>* : loads <directory> as a configuration files. The default one is `/usr/local/etc/Marcel`
* *-t* : test configuration file and exit

## Simulation mode

When running in "**Simulation mode**", all sections with the "`DoNotSimulate`" flag set are considered disabled.

## Logging
As of version 6.05, Marcel publishes its logs on the following topics : 
* **%*MarcelID*%/Log/Fatal** : Failures causing Marcel to stop or major functionality loss
* **%*MarcelID*%/Log/Error** : Something went wrong, but it didn't impact Marcel's health
* **%*MarcelID*%/Log/Warning** : something you must be aware of 
* **%*MarcelID*%/Log/Information** : Startup steps and running information
* **%*MarcelID*%/Log** : Trace information (incoming messages, decisions, etc …)

As of version 7.07, the following topics have been added (if **mod_alert** enabled) :
* **%*MarcelID*%/Log/Error** : send raising alerts as well (*same as Marcel's own errors*)
* **%*MarcelID*%/Log/Corrected** : send corrected alerts
* **%*MarcelID*%/AlertsCounter** : amount of active alerts

## Status change
### Section
As of 8.3, Marcel reports section status changes to topic<br>
**%*MarcelID*%/Change/<Section ID>**<br>
and the payload contains 2 fields:
- Enabled(`1`) or Disabled (`0`)
- face technical error

As example, if `Marcel/Change/Info` is published with
```
1,0
```
means, **Info** section is *enabled* and *doesn't face any errors*.

### Named Notification (only if mod_alert is loaded)
As of 8.3, Marcel reports Named Notification status changes to topic<br>
**%*MarcelID*%/NamedNotificationChange/<Section ID>**<br>
and the payload contains 1 field:
- Enabled(`1`) or Disabled (`0`)

As example, if `Marcel/NamedNotificationChange/p` is published with
```
1
```
means, **p** named notification is *enabled*.

## Modules

`Modules` contains the documentation for each module.<br>
Taking a look in `Modules/Marcel` is a *must-read* about global directives.
  
## Side note
The name is a tribute to my late rabbit, who passed away some days before I started this project : he stayed at home as a keeper. RIP.
