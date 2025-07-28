mod_sht31
====

Exposes SHT31 (Temperature/humidity probe) figures. This probe is plugged into one of the local I2C buses.

### Accepted global directives

none

## Section SHT31
### Accepted directives

* **Topic=** Topic to publish to. `%FIGURE%` will be replaced by Temperature and Humidity as per the submitted value.
* **Device=** I2C device
* **Address=** Probe I2C address (if not set, default to : 0x44)
* **Sample=** Number of seconds between samples, in seconds
* **Immediate** Launch the first sample at startup
* **Keep** Don't abort in case of technical error
* **func=** Acceptation function (*see bellow*, **mod_Lua** needed)
* **OffsetT=** offset to add to temperature value
* **OffsetH=** offset to add to humidity value
* **Disabled** Start this section disabled
* **DoNotSimulate** disables this section when running in simulation mode.

### Lua function arguments

1. Section ID
2. Temperature (°C)
3. Humidity (%)

### Lua function return

1. `true` if values are accepted and so, published

## Error condition

An error condition is associated with each section, individually. It is raised if a technical issue prevents reading data, and is cleared as soon as an attempt succeeds.

Error condition is exposed to Lua by **SHT31:inError()**.
