mod_axp20x
====

Exposes AXP20x PMU (Power Management Unit) figures.

### Accepted global directives

none

## Section AXP20x
### Accepted directives

* **Topic=** Topic to publish to. `%FIGURE%` will be replaced by the figure listed bellow
* **Figures=** can be `ac`, `vbus`, `ips`, `temp` (`bat` is not yet supported)
* **Device=** I2C device
* **Address=** Probe I2C address (if not set, default to : 0x34)
* **Sample=** Number of seconds between samples, in seconds
* **Immediate** Launch the first sample at startup
* **Keep** Don't abort in case of technical error (only if the chip can't be reached, reading errors are always considered as non fatal)
* **func=** Acceptation function (*see bellow*, **mod_Lua** needed)
* **Disabled** Start this section disabled
* **DoNotSimulate** disables this section when running in simulation mode.

### Lua function arguments

1. Section ID
2. Figure's name (can be "AC", "VBus", "IPS" or "Temperature")
3. Voltage (V) or temperature (°C)
4. Current (mA), `nil` for IPS and temperature.

### Lua function return

1. `true` if values are accepted and so, published

## Error condition

An error condition is associated with each section, individually. It is raised if a technical issue prevents reading data, and is cleared as soon as an attempt succeeds.

Error condition is exposed to Lua by **AXP20X:inError()**.
