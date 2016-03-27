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
* **-DFREEBOX** : Enable [Freebox](https://en.wikipedia.org/wiki/Freebox) v4/v5 figures publishing (French **I**nternet **S**ervice **P**rovider)
* **-DUPS** : Enable UPS monitoring. Query [NUT](http://www.networkupstools.org/) through its Telnet interface to retrieve UPS figures.
* **-DLUA** : Enable Lua user function (v3.0+)
* **-DMETEO** : Enable meteo forcast publishing (using [OpenWeatherMap.org](http://www.OpenWeatherMap.org))

##Note :##
This Makefile is automatically generated by my old (but still useful :) ) [LFMakeMaker](http://destroyedlolo.info/Developpement/LFMakeMaker/).
I strongly suggest to modify then launch **remake.sh** instead of tedious Makefile changes.

#Configuration :#
By default, Marcel reads **/usr/local/etc/Marcel.conf** as configuration file (may be changed using command line option **-f**).
Have a look on provided file which contains comprehensive explanation of known directives.

#Lua custom methods :#
Following methods are exposed to Lua code through **Marcel** object :
* **Marcel.MQTTPublish(** topic, value **)** : Publish a value to MQTT broker

* **Marcel.RiseAlert(** topic**,** message **)**, **Marcel.RiseAlertSMS(** topic**,** message **)** : Tells Marcel about an alert condition
* **Marcel.ClearAlert(** topic **)** : Clear an alert condition

* **Marcel.SendMessage(** title **,** Text **)** and **Marcel.SendMessageSMS(** title **,** Text **)**: The 1st one sends a message using **AlertCommand facility** which is generally used to send a mail. The 2nd one uses also **SMSUrl** which is generally used to send an SMS.
* **Marcel.SendNamedMessage(** names **,** title **,** Text **)**: Sends a named notification (see bellow).

* **Marcel.Hostname()** : As the name said, host's name
* **Marcel.ClientID()** : Configured MQTT client id
* **Marcel.Version()** : Marcel's version (*have a look on **scripts/AllVerif.lua** for an usage example*)

**Notez-bien :** Marcel is able run only **one Lua function at once**. Consequently, your functions have to be as fast as possible.

#Installation :#
A startup script for OpenRC has been provided in sub-directory ... startup_scripts.

#Launch options :#
Marcel knows the following options :
* *-h* : online help
* *-v* : verbose output
* *-f<file>* : loads <file> as configuration file. The default one is `/usr/local/etc/Marcel.conf`
* *-t* : test configuration file and exit

Have a look on provided configuration file to guess the syntax used (I'm busy, a full documentation will come later).

#Alerts vs Notifications
* Alerts respond to ' **Alert/sub topic/message**'
* Notifications respond to ' **Notification/sub topic/message**'

If the first character of *message* is an '**S**' or '**s**' it's meaning an alert is raising and a communication will be send only if it's not an already *open* alert.
A *message* not starting with  '**S**' or '**s**' means the alert is closing.

Unlike Alerts, Notifications are not checked against duplication : in other words, communication are unconditionally sent.

* **S** : both SMS and Mail will be send
* **s** : only Mail will be send

#Named notifications#
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

#Side note#
The name is a tribute to my late rabbit that passed away some days before I did started this project : he stayed at home as keeper. RIP.
> Written with [StackEdit](https://stackedit.io/).
