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

* **TaHoma_host=** TaHoma hostname
* **TaHoma_address=** TaHoma IP address
* **TaHoma_port=** TaHoma port (`8443` by default)
* **TaHoma_token=** Application token as provided by the mobile application

* **DontVerifySSL** SSL chain enforcement is not required.
In other words, you don't need to add Overkiz's root certificate to your repository.

# Build dependancies

This module requires `libcurl` and `libjson-c` libraries.

