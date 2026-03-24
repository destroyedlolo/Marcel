Marcel
===
**Marcel** is a Lightweight, Open, Powerful, and Truly Modular **data gathering and MQTT publisher**.

# Introduction to Marcel ?

**Marcel** is a lightweight daemon designed to collect, process, and publish a broad spectrum of data. Its modular architecture allows you to:

* **Acquire environmental data** from various sources, such as 1-Wire probes or third-party gateways (e.g., Somfy TaHoma), and drive connected actuators.
* **Fetch and publish** local weather forecasts.
* **Monitor UPS** (Uninterruptible Power Supply) metrics and generate corresponding alerts.
* **Integrate** many other data sources and services
* and many more ...

## Built for Flexibility, Power, and Efficiency

Adhering to **KISS** (Keep It Simple, Stupid) principles, Marcel features a highly modular architecture composed of dedicated, reliable, and
independent modules.  
This design ensures maximum efficiency: you only load the specific modules required for your use case, eliminating bloat and preventing resource
waste on unused functionalities.

Thanks to its open and powerful module's API, it's *easy* to add new functionalities.

## Data Quality and Error Management

**Marcel** can monitor external MQTT events to ensure timely completion, data quality, and trustworthiness. It can raise alerts if a deviance is detected.  
The system embeds a simple but powerful mechanism to manage and communicate throughout an issue's lifecycle, from initial detection to final resolution.


## Maximum Data Mastering

Incoming data can be validated and enhanced utilizing the system's powerful **Lua** scripting capabilities. This process is also known as **Brown to Silver** data transformation.

# Installation

## From sources

If **Marcel** is not packaged for your distribution, the `Build from source.md` file contains the technical information required to compile it directly from source code.

> [!TIP]
> **Marcel** is primarily designed and developed for **Linux**. It has been intensively tested on **Linux x86, AMD64, and ARM** architectures.  
> While there is no known technical barrier to running it on **BSD** systems, it is not officially tested or supported on that platform (yet ?).

## Dependencies

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

### mod_TaHoma own's

[json-c](https://github.com/json-c/json-c/wiki) and [libcurl](https://curl.se/libcurl/) are needed to communicate with your TaHoma.

# Running

## Configuration

The `Modules` directory contains specific documentation for each module, along with configuration examples. Comprehensive use-case examples are currently in development and will be available soon.

Before diving into specific modules, consulting the `Modules/Marcel` documentation is **essential reading** to understand global configuration directives.

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
  
# Side note
The name is a tribute to my late rabbit, who passed away some days before I started this project : he stayed at home as a keeper. RIP.
