mod_TaHoma
===

Bridge your **TaHoma Switch** (or derivated) with **MQTT** using a very low footprint.

**TaHoma** enhances your local automation by supporting a broad range of local devices, including **RTS**, **IO-homecontrol**, **Zigbee**, and soon **Matter** or **KNX**.  
However, as this setup relies solely on the local API, cloud-based feature, such as Cloud-to-Cloud connectivity (like Somfy Protect), TaHoma-managed scenarios, and more - will not be
accessible.

# Setup

Use **[TaHomaCtl](https://github.com/destroyedlolo/TaHomaCtl)** companion as per the following procedure :

### Enable the TaHoma local API

You need the TaHoma's developer mode activated : follow [Overkiz's intruction](https://github.com/Somfy-Developer/Somfy-TaHoma-Developer-Mode).

### discover your box

```bash
$ ./TaHomaCtl -vU
*W* SSL chaine not enforced (unsafe mode)
TaHomaCtl > scan_TaHoma
*I* Service 'gateway-xxxx-xxxx-xxxx' of type '_kizboxdev._tcp' in domain 'local':
TaHomaCtl > status
*I* Connection :
        Tahoma's host : gateway-xxxx-xxxx-xxxx.local
        Tahoma's IP : 192.168.0.30
        Tahoma's port : 8443
        Token : set
        SSL chaine : not checked (unsafe)
*I* 0 Stored device
```

### discover your devices

```
TaHomaCtl > scan_Devices 
*I* 15 devices
TaHomaCtl > Device
test_air : zigbee://xxxx-xxxx-xxxx/58849/1#1
test_air : zigbee://xxxx-xxxx-xxxx/58849/1#2
test_air : zigbee://xxxx-xxxx-xxxx/58849/3
test_air : zigbee://xxxx-xxxx-xxxx/58849/1#4
Deco : io://xxxx-xxxx-xxxx/5335270
Porte_Chat : rts://xxxx-xxxx-xxxx/16774417
IO_(10069463) : io://xxxx-xxxx-xxxx/10069463
test_air : zigbee://xxxx-xxxx-xxxx/58849/0
ZIGBEE_(0/0) : zigbee://xxxx-xxxx-xxxx/0/0
Boiboite : internal://xxxx-xxxx-xxxx/pod/0
INTERNAL_(wifi/0) : internal://xxxx-xxxx-xxxx/wifi/0
test_air : zigbee://xxxx-xxxx-xxxx/58849/1#3
ZIGBEE_(0/242) : zigbee://xxxx-xxxx-xxxx/0/242
ZIGBEE_(0/1) : zigbee://xxxx-xxxx-xxxx/0/1
ZIGBEE_(65535) : zigbee://xxxx-xxxx-xxxx/65535
```
and for each of them, the states you can retrieve and the command you can send :
```
TaHomaCtl > Device io://xxxx-xxxx-xxxx/5335270
Deco : io://xxxx-xxxx-xxxx/5335270
	Commands
		toggle (0 arg)
		resetLockLevels (0 arg)
		advancedRefresh (1 arg)
		setConfigState (1 arg)
		unpairAllOneWayControllers (0 arg)
		onWithTimer (1 arg)
		wink (1 arg)
		addLockLevel (1 arg)
		removeLockLevel (1 arg)
		setOnOff (1 arg)
		off (0 arg)
		on (0 arg)
		setName (1 arg)
		identify (0 arg)
		getName (0 arg)
		delayedStopIdentify (1 arg)
		pairOneWayController (1 arg)
		startIdentify (0 arg)
		stopIdentify (0 arg)
		unpairOneWayController (1 arg)
	States
		core:OnOffState
		io:PriorityLockOriginatorState
		io:PriorityLockLevelState
		core:PriorityLockTimerState
		core:RSSILevelState
		core:DiscreteRSSILevelState
		core:NameState
		core:CommandLockLevelsState
		core:StatusState
```
Then, you can configure mod_TaHoma as explained bellow.

# Configuration
### Accepted global directives

None

## Section TaHoma=

Describe your gateway as discovered above.

> [!TIP]
> Marcel can manage multiple TaHoma devices simultaneously, each defined in a separate section.
> The only requirement is to assign a unique identifier to each device (specified in the `TaHoma=` parameter).

* **TaHoma_host=** TaHoma hostname
* **TaHoma_address=** TaHoma IP address
* **TaHoma_port=** TaHoma port (`8443` by default)
* **TaHoma_token=** Application token as provided by the mobile application

* **DontVerifySSL** SSL chain enforcement is not required.
In other words, you don't need to add Overkiz's root certificate to your repository.

* **Sample=** Delay (in seconds) between asking for events.

### Subsection **Expect= : listening TaHoma's events

TaHoma notifies users about changes or the delivery of figures through **events**, which is far more efficient
than polling for probe figures (as explained later). The Expect= subsections allow you to specify the type of events
you are interested in.

* **expect_Event=** the event's kind your looking for (i.e : `DeviceStateChangedEvent`)
* **expect_deviceURL=** URL of the device (`zigbee://xxxx-xxxx-xxxx/58849/1#2`)
* **expect_state=** state to consider (`core:RelativeHumidityState`)

When an event matches the criteria described above, an MQTT message is issued.

* **expect_Topic=** Topic to publish to
* **expect_Retained** A retained message

> [!IMPORTANT]
> Whatever the kind exposed by the TaHoma, the figure is published as a string using `json_object_to_json_string()`.

* **expect_Disabled** this expectation is initially disabled
* **expect_DoNotSimulate** this expection is ignored when Marcel is running in "Simulate" mode

## Section Probe=

The **Probe** section is used to retrieve state values by polling the TaHoma local API.

* **url=** Device's URL
* **TaHoma=** Gateway it belongs to
* **Sample=** Delay between samples (`-1` : Query once upon startup)
* **Immediate** Launch it immediately; otherwise, the first query will occur after the delay specified by `Sample=`.
* **Disabled** starts as disabled
* **DoNotSimulate** disable it if Marcel is running in "Simulate" mode.

### Subsection State=

The **State=** subsection allows you to specify which state to look for and how to submit it.

* **state_Topic=** Topic to publish to
* **state_Retained** a retained message
* **state_Disabled** this state is initially disabled
* **state_DoNotSimulate** this state is ignored when Marcel is running in "Simulate" mode

> [!TIP]
> The most efficient way to implement querying is as follows:
> - Create Probe sections with Sample=-1 to retrieve the initial value at startup.
> - In the TaHoma definition, use the Expect= parameter to process events corresponding to your target probes.
> 
> Naturally, the same Topic must be specified for both.

# Build dependancies

This module requires `libcurl` and `libjson-c` libraries.

