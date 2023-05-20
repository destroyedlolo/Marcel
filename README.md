Marcel
===
**Marcel** is a lightweight versatile **MQTT data publisher**

# Dependancies

As the communication is based on MQTT messages, you obviously need a ... **broker** :
I personally use [Mosquitto](http://mosquitto.org/).

## Modules related runtime dependancies

### 1-wire (mod_1wire)
Recent linux kernel has *better than nothing* and *limited* 1-wire support. I strongly suggest to use [OWFS](https://www.owfs.org/) instead.

### OpenWeatherMap (mod_OpenWeatherMap)
You need to provide your own license key to query weather forecast, it's free for hobbyists usages.<br>
Have a look on : https://openweathermap.org/


## Launch options :
Marcel knows the following options :
* *-h* : online help
* *-v* : verbose output
* *-f<file>* : loads <file> as configuration file. The default one is `/usr/local/etc/Marcel.conf`
* *-t* : test configuration file and exit

## Logging
As of version 6.05, Marcel publishes its loggings to following topics : 
* **%*MarcelID*%/Log/Fatal** : Failures causing Marcel to stop or major functionality loss
* **%*MarcelID*%/Log/Error** : something went wrong but it didn't impacted Marcel's health
* **%*MarcelID*%/Log/Warning** : something you must be aware of 
* **%*MarcelID*%/Log/Information** : Startup steps and running informations
* **%*MarcelID*%/Log** : Trace information (incoming messages, decisions, etc ..)

As of version 7.07, following topics have been added (if **mod_alert** enabled) :
* **%*MarcelID*%/Log/Error** : send raising alerts as well (*same as Marcel's own errors*)
* **%*MarcelID*%/Log/Corrected** : send corrected alerts
* **%*MarcelID*%/AlertsCounter** : amount of active alerts

## Modules

`Modules` containes the documentation of each modules.<br>
Having a look in `Modules/Marcel` is a *must read* about global directives.
  
## Side note
The name is a tribute to my late rabbit that passed away some days before I did started this project : he stayed at home as keeper. RIP.
