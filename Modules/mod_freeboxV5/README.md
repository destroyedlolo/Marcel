mod_freeboxV5
====

Publish Freebox v4/v5 figures (French Internet Service Provider)

| :warning:WARNING:warning: : This module is not supported (as I don't have the suitable Freebox anymore) |
| --- |

### Accepted global directives
none

## Section Freebox

### Accepted directives
* **Sample=** Number of seconds between samples, in seconds
* **Topic=** root of topics
* **Keep** Do not die in case of error, sleep until next run [optional]
* **Immediate** Execute at startup (or when the section is enabled) then wait for *sample* seconds [optional]
* **Disabled** Section is disabled at startup [optional]

## Example

An example is provided in `Config` sub directory of the current one
