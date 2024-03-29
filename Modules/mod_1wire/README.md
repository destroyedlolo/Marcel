mod_1wire
====

Handles 1-wire probes (also suitable for any value exposed as a file).

### Accepted global directives

* **DefaultSampleDelay=** Sample value to apply when `Sample=` is absent. This directive is convenient to 
provide the same sample value to many probes.<br>
Many `DefaultSampleDelay=` can be present, in such case the last one before the section definition is
taken into account.

* **RandomizeProbes** FFV will be randomly delayed to avoid each probes to samples at the same time 
(and then creates electricity consumption pick, really needed for very large network).

## Section FFV

*Float value exposed as a file*.<br>
Mostly provided for 1-wire probes but can be used for any value exposed as a file 
(like motherboard's health probes). In addition, a function
callback can be used to parse complex files (needs **mod_Lua**).

### Accepted directives

* **File=** Where the probe is exposed, file to read
* **Func=** Acceptance function (**mod_Lua** needed)
* **FailFunc=** Callback in case of technical failure (**mod_Lua** needed)
* **Topic=** Topic to publish to
* **Retained** submitted as retained message
* **Offset=** offset to add to temperature value (*for calibration purpose*)
* **Safe85** if the read value is 85, data is not published and an 'E' error
is raised. Mostly applicable to 1-wire temperature probes where 85° means
the probe is underpowered
* **Sample=** Number of seconds between samples, in seconds. Special values are :
  * missing : takes the last `DefaultSampleDelay=`
  * **-1** run only once
* **Immediate** Launch the 1st sample at startup

## Section 1WAlarm

Like FFV but value is read only when the probe is in alarm status

### Accepted directives

* **File=** Where the probe is exposed
* **Latch=** Write to this file to reset the probe's latch (and alarm)
* **Func=** Acceptance function (**mod_Lua** needed)
* **FailFunc=** Callback in case of technical failure (**mod_Lua** needed)
* **InitFunc=** Function to be executed at startup (Typical usage : set the probe in Alert mode, **mod_Lua** needed))
* **Topic=** Topic to publish to
* **Retained** submitted as retained message
* **Immediate** Read and publish value at launching

## Error condition

An error condition is associated to each section, individually. It is raised if :
- a technical issue prevents to read the probe
- if the probe is under powered and **safe85** is set. But in such case, **FailFunc** is not called.

The error condition is cleared as soon as a successful attempt to read the probe.

Error condition is exposed to Lua by **FFV:inError()** and **1WAlarm:inError()** methods.

## Lua interface
### Func
#### Arguments

1. Section id
2. Topic
3. raw value
4. compensated value

#### Return

`true` if the value has to be published, `false` if not

### InitFunc
#### Arguments

1. Section id
2. File

### FailFunc
#### Arguments

1. Section id
2. error message

## Objects exposed to Lua
### Exposed functions

- **mod_1wire.AlarmInError()** - (boolean) return if 1-wire Alarm is facing a technical error (like alarm directory missing).
