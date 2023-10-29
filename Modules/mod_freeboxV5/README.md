mod_freeboxV5
====

Publish Freebox v4/v5 figures (French Internet Service Provider)

| :warning:WARNING:warning: : This module is not supported (as I don't have the suitable Freebox anymore) |
| --- |

### Accepted global directives
none

## Section FreeboxV5

### Accepted directives
* **Sample=** Number of seconds between samples, in seconds
* **Topic=** root of topics
* **Keep** Do not die in case of error, sleep until next run [optional]
* **Immediate** Execute at startup (or when the section is enabled) then wait for *sample* seconds [optional]
* **Disabled** Section is disabled at startup [optional]

## Example

An example is provided in `Config` sub directory of the current one

## Error condition

An error condition is associated to each section, individually. It is raised if a technical issue prevents to read data and is cleared as soon as an attempt succeed.

Error condition is exposed to Lua by **FreeboxV5.inError()** method.
