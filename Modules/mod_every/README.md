mod_every
====

Repeating tasks (**mod_Lua** is obviously required).

### Global directives
none

## Section Every
Repeat a function *every* seconds.

### Directives
* **Sample=** Number of seconds between launch, in seconds
* **Func=** Function to execute
* **Topic=** passed as argument (see bellow) [optional]
* **Immediate** Execute at startup (or when the section is enabled) then wait for *sample* seconds [optional]
* **Disabled** Section is disabled at startup [optional]

### Argument

* **Topic=**'s content if provided, the section name otherwise.

## Section At
Launch a function at the given time, daily.

### Directives
* **At=** At which time the function will be launched (format HHMM so `1425` means it will be launched at 2.25PM)
* **Func=** Function to execute
* **Topic=** passed as argument (see bellow) [optional]
* **Immediate** Execute at startup (or when the section is enabled) then wait for the given hour [optional]
* **RunIfOver** Execute the function if the specified hour is over [optional]
* **Disabled** Section is disabled at startup [optional]

## Example

An example is provided in `Config` subdirectory
