# mod_1wire
Handles 1-wire probes (also suitable for any value exposed as a file).

### Accepted global directives

* **DefaultSampleDelay=** Sample value to apply when Sample=0 or absent. This directive is convenient to 
provide the same sample value to many probes.<br>
Many DefaultSampleDelay= can be present, in such case the last one before the section definition is
taken into account.

## Section FFV

*Float value exposed as a file*.<br>
Mostly provided for 1-wire probes but can be used for any value exposed as a file 
(like motherboard's health probes). In such case, 1-wire only options are useless. In addition, a function
callback can be used to parse complex files.

### Accepted directives

* **File=** Where the probe is exposed
* **Func=** Acceptance function
* **FailFunc=** Callback in case of technical failure
* **Topic=** Topic to publish to
* **Retained** submitted as retained message
* **Offset=** offset to add to temperature value (*for calibration purpose*)
* **Safe85** if the read value is 85, data is not published and an 'E' error
is raised. Mostly applicable to 1-wire temperature probes where 85° means
the probe is underpowered
* **Sample=** Number of seconds between samples, in seconds. Special values are :
  * **0** or missing : takes the last `DefaultSampleDelay=`
  * **-1** run only once
* **Immediate** Launch the 1st sample at startup
