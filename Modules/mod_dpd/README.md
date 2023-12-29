mod_dpd
====

**D**ead **P**ublisher **D**etector : Alerts if some figures don't come in time.

### Accepted global directives
none

## Section DPD

### Accepted directives

*  **Topic=** Topic to listen to
*  **Timeout=** Watchdog timeout in seconds (**Sample=** is also accepted)  [optional, *see below*]
*  **NotificationTopic=** where to notify in case of watchdog
*  **Func=** Acceptation function, if returns 'false', watchdog continue [optional, *see below*]
*  **Disabled** Section is disabled at startup [optional]
*  **Keep** using `Keep` prevents Marcel to crash in case of technical error. 
But in such case, it will continue in DEGRADED way, with notifying when a message is received.

At least one of `Timeout=` and `Func=` must be present, otherwise the section is useless and will die.

#### Lua function arguments

1. Section ID
2. Topic
3. MQTT message value

#### Lua function return

1. `true` if the value is accepted and the watchdog reset. `false`, value rejected.

### Lua specific method

* **inError()** - returns if DPD is in error state (boolean)

# Provided examples

In addition to the basic `70_dpd` example, 2 usefull DPDs are provided (I'm using them for my dashboard)

### 70_Info
If a message is received on `%ClientID%/About` topic, Marcel's version and copyright.
```
Marcel's version 8.0107
Marcel v8.0107 (c) L.Faillie 2015-2023
```
### 70_Status
If a message is received on `%ClientID%/Status` topic, the real-time status of sections are issued to `%ClientID%/Status/response`
```
Marcel status	DPD	1	0
Marcel info	DPD	1	0
onduleur	UPS	0	1
```

Fields are :

- Section's name
- Is section enabled (1) or not (0)
- optionaly, is section in error

### 70_namedNotificationStatus
If a message is received on `%ClientID%/NamedNotificationsStatus` topic, the real-time status of sections are issued to `%ClientID%/NamedNotificationsStatus/response`
```
Name	Enable
------
i	0
E	1
```

Fields are :

- Named Notification's name
- Is it enabled (1) or not (0)
