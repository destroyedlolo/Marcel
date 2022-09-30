# mod_ups

Get UPS informations from a NUT server

### Accepted global directives
none

## Section UPS

:exclamation:Notez-bien:exclamation: : Section ID is the identifier of the UPS in NUT server.

### Accepted directives
* **Sample=** Number of seconds between samples, in seconds
* **Func=** Function to execute
* **Topic=** root of topics
* **Keep** Do not die in case of error, sleep until next run [optional]
* **Disabled** Section is disabled at startup [optional]


## Example

An example is provided in `Config` sub directory of the current one

With configuration like
```
*UPS=onduleur
	Topic=onduleur.dev
	Var=ups.load
```

current UPS load with be published as `onduleur.dev/ups.load`
