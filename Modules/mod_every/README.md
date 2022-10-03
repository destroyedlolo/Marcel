# mod_every

Repeating tasks

### Accepted global directives
none

## Section Every
Repeat a function *every* seconds.

### Accepted directives
* **Sample=** Number of seconds between samples, in seconds
* **Func=** Function to execute
* **Topic=** passed as argument (see bellow) [optional]
* **Immediate** Execute at startup (or when the section is enabled) then wait for *sample* seconds [optional]
* **Disabled** Section is disabled at startup [optional]

### Argument

* **Topic=**'s content if provided, the section name otherwise.

## Section At
Launch a function at the given time, daily.

### Accepted directives
* **At=** At which time the function will be launched (format HHMM so `1425` means it will be launched at 2.25PM)
* **Func=** Function to execute
* **Topic=** passed as argument (see bellow) [optional]
* **Immediate** Execute at startup (or when the section is enabled) then wait for the given hour [optional]
* **RunIfOver** Execute the function if the specified hour is over [optional]
* **Disabled** Section is disabled at startup [optional]

## Example

An example is provided in `Config` sub directory of the current one
