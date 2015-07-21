# Marcel

**Marcel** is a versatile daemon to publish easily some data to an **MQTT broker**.

#Requirements :#
* MQTT broker (I personally use [Mosquitto](http://mosquitto.org/) )
* [Paho](http://eclipse.org/paho/) as MQTT communication layer.

#Installation :#
* Get **Marcel.c** and put it in a temporary directory
* Install **PAHO**
* Compile Marcel using the following command line :

    gcc -std=c99 -lpthread -lpaho-mqtt3c -Wall Marcel.c -o Marcel

Add following options to include related modules :
* -DFREEBOX : Enable Freebox statistics (French Internet provider)
* -DUPS : Enable UPS monitoring, NUT needed
* -DLUA : Enable Lua user function (v3.0+)

#Launch options :#
Marcel knows the following options :
* *-v* : verbose output
* *-f<file>* : loads <file> as configuration file. The default one is `/usr/local/etc/Marcel.conf`

Have a look on provided configuration file to guess the syntax used (I'm busy, a full documentation will come later).

#Side note#
The name is a tribute to my late rabbit that passed away some days before I did started this project : he stayed at home to keep our house. RIP.
> Written with [StackEdit](https://stackedit.io/).
