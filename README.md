# Marcel
**Marcel** is a lightweight versatile daemon to
- publish easily some figures to an **MQTT broker** (1wire probe values, UPS, ...)
- watchdog on published data 
- custom checks can be implemented through Lua scripts
- finally send SMS, mail or publish alert topic in case a test failed and then when it recovers.

## Requirements :
* MQTT broker (I personally use [Mosquitto](http://mosquitto.org/) )
* [Paho](http://eclipse.org/paho/) as MQTT communication layer.
* [json-c](https://github.com/json-c/json-c/wiki) library

## Compilation :
* Get **Marcel** latest tarball for *stable* version or clone its repository for *development* version
* Install **PAHO** library for C ( https://eclipse.org/paho/clients/c/ )
* **curl** is needed as well to send SMS
* Compile Marcel

    make

You may modify the **Makefile** to add or remove following compiler options :
* **-DFREEBOX** : Enable [Freebox](https://en.wikipedia.org/wiki/Freebox) v4/v5 figures publishing (French **I**nternet **S**ervice **P**rovider) (v1.1+)
* **-DUPS** : Enable UPS monitoring. Query [NUT](http://www.networkupstools.org/) through its Telnet interface to retrieve UPS figures. (v1.2+)
* **-DLUA** : Enable Lua user function (v3.0+)
* **-DMETEO** : Enable meteo forcast publishing (using [OpenWeatherMap.org](http://www.OpenWeatherMap.org)) (v4.1+)
* **-DRFXTRX** : Add support of [RFXCom transceiver](http://www.rfxcom.com/) (v4.7+)
* **-DSHT31** : Add support of [SHT31 Humidity sensor](https://www.sensirion.com/en/environmental-sensors/humidity-sensors/digital-humidity-sensors-for-various-applications/) (7.06+)

**Note :**
This Makefile is automatically generated by my old (but still useful :) ) [LFMakeMaker](http://destroyedlolo.info/Developpement/LFMakeMaker/).
I strongly suggest to modify then launch **remake.sh** instead of tedious Makefile changes.

## Installation :
A startup script for OpenRC has been provided in sub-directory ... startup_scripts.

## Launch options :
Marcel knows the following options :
* *-h* : online help
* *-v* : verbose output
* *-f<file>* : loads <file> as configuration file. The default one is `/usr/local/etc/Marcel.conf`
* *-t* : test configuration file and exit

Have a look on provided configuration file to guess the syntax used (I'm busy, a full documentation will come later).

## Configuration :
By default, Marcel reads **/usr/local/etc/Marcel.conf** as configuration file (may be changed using command line option **-f**).
Have a look on provided file which contains comprehensive explanations of all known directives.

**Notez-bien :**
Starting v6.0+, each sections in the configuration file must be uniquely named.

## Lua custom methods :
Following methods are exposed to Lua code through **Marcel** object :
* **Marcel.MQTTPublish(** topic, value [, retain] **)** : Publish a value to MQTT broker.
If *retain* = true, the message is kept.

* **Marcel.RiseAlert(** topic **,** message **)**, **Marcel.RiseAlertSMS(** topic **,** message **)** : Tells Marcel about an alert condition
* **Marcel.ClearAlert(** topic **)** : Clear an alert condition

* **Marcel.SendMessage(** title **,** Text **)** and **Marcel.SendMessageSMS(** title **,** Text **)**: The 1st one sends a message using **AlertCommand facility** which is generally used to send a mail. The 2nd one uses also **SMSUrl** which is generally used to send an SMS.
* **Marcel.SendNamedMessage(** names **,** title **,** Text **)**: Sends a named notification (see bellow).

* **Marcel.Hostname()** : As the name said, host's name
* **Marcel.ClientID()** : Configured MQTT client id
* **Marcel.Version()** : Marcel's version (have a look on **scripts/AllVerif.lua** for an usage example)

**Notez-bien :** Marcel is able run only **one Lua function at once**. Consequently, your functions have to be as fast as possible.

## Enable / Disable a section

Starting v6.03, it is possible to enable / disable individually sections.

To do that, send to *MarcelID*`/OnOff/`*section_name* :
* **0**, **Off**, **Disable** : Disable corresponding section
* *any other value* : Enable corresponding section

## Alerts vs Notifications
* Alerts respond to '**Alert/...**' topics
* Notifications respond to '**Notification/...**' topics

If the first character of the payload is an '**S**' or '**s**' it's meaning an alert is raising and a communication will be send only if it's not an already *open* alert.
A payload not starting with  '**S**' or '**s**' means the alert is closing.

Unlike Alerts, Notifications are not checked against duplication : in other words, communication are unconditionally sent.

* **S** : both SMS and Mail will be send
* **s** : only Mail will be send

## Named notifications
Named notifications are declared using `$alert=` configuration directive as

    $alert=N
    SMSUrl=http://api.pushingbox.com/pushingbox?devid=xxxxxxxxxxx&msg=%s
    AlertCommand=mail -s "%t%" mail@domain.com

The argument of the section, a single character, is the **name** of the alert. In the example above, it's '**N**'.
It has to be followed with an **SMSUrl=** or **AlertCommand=** describing the action(s) to do. Have a look on provided *Marcel.conf* to see the syntax use. Notez-bien, both directives can be present and in this case, both actions are done.

Named notifications are raised using
* Lua's **Marcel.SendNamedMessage(** *names* **,** *title* **,** *Text* **)**
* sending a message to `nNotification/`*names*`/`*title* topic.

*names* is a string on which each and every characters correspond to the name of an alert to raise. As example, if *names*=`ABC` means that alerts **A**, **B** and **C** will be sent.

## Logging
As of version 6.05, Marcel publishes its loggings to following topics : 
* **< *MarcelID* >/Log/Fatal** : Failures causing Marcel to stop or major functionality loss
* **< *MarcelID* >/Log/Error** : something went wrong but it didn't impacted Marcel's health
* **< *MarcelID* >/Log/Warning** : something you must be aware of 
* **< *MarcelID* >/Log/Information** : Startup steps and running informations
* **< *MarcelID* >/Log** : Trace information (incoming messages, decisions, etc ..)

As of version 7.07, following topics have been added :
* **< *MarcelID* >/Log/Error** : send raising alerts as well (*same as Marcel's own errors*)
* **< *MarcelID* >/Log/Corrected** : send corrected alerts
* **< *MarcelID* >/AlertsCounter** : amount of active alerts

## Side note
The name is a tribute to my late rabbit that passed away some days before I did started this project : he stayed at home as keeper. RIP.

