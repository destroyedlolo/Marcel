# mod_every

Repeating tasks

### Accepted global directives
none

## Section Every
Repeat a function *every* seconds.

### Accepted directives
* **Sample=** Number of seconds between samples, in seconds
* **Func=** Function to execute
* **Immediate** Execute at startup (or when the section is enabled) then wait for *sample* seconds
* **Disabled** Section is disabled at startup

## Section At
Launch a function at the given time, daily.

### Accepted directives
* **At=** At which time the function will be launched (format HHMM so `1425` means it will be launched at 2.25PM)
* **Func=** Function to execute
* **RunIfOver** Execute the function if the specified hour is over
* **Disabled** Section is disabled at startup
