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

### Lua function arguments

1. Section ID
2. Topic
3. MQTT message value

### Lua function return

1. `true` if the value is accepted and the watchdog reset. `false`, value rejected.
