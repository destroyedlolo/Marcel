mod_every
====

Repeating tasks (**mod_Lua** is obviously required).

### Global directives
none

## Section Every
Repeat a function *every* second.

### Directives
* **Sample=** Number of seconds between launches, in seconds
* **Func=** Function to execute
* **Arg=** Argument to pass as 2nd argument to the function [optional]
* **Topic=** passed as an argument (see below) [optional]
* **Immediate** Executes at startup (or when the section is enabled), then waiting for *sample* seconds [optional]
* **Disabled** Section is disabled at startup [optional]
*  **DoNotSimulate** disables this section when running in simulation mode.

### Argument

* **Topic=**'s content if provided, the section name otherwise.

## Section At
Launch a function at the given time, daily.

### Directives
* **At=** At which time the function will be launched (format HHMM, so `1425` means it will be launched at 2.25PM)
* **Func=** Function to execute
* **Arg=** Argument to pass as 2nd argument to the function [optional]
* **Topic=** passed as argument (see bellow) [optional]
* **Immediate** Execute at startup (or when the section is enabled), then wait for the given hour [optional]
* **RunIfOver** Execute the function if the specified hour is over [optional]
* **Disabled** Section is disabled at startup [optional]

## Example

An example is provided in the `Config` subdirectory

