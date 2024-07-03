As V8 deeply broke syntax compatibility with the previous version, here are some notes I took when I switched my own installation.

# Compilation

If facing some MQTT-related linking errors during compilation, upgrade **paho** library.

# Global settings 

- ` ConnectionLostIsFatal` - Doesn't exist anymore, as it's now the default behavior.
I gave up implementing automatic reconnect; it's far better to handle it externally within the system's startup mechanism.

# Modules' specifics

## mod_alert

- `$notification=` (and its alias `$alert=`)  are deprecated. They are replaced by `$namedNotification=` which share the same syntax and usage.<br>
Notez-bien : `$alert` (without trailing '=') declares an unnamed alert.
- `SMSUrl=` and `AlertCommand=` are deprecated.
- `OSCmd=` and `RESTUrl=` are no longer accepted as global directives. To  achieve the same behavior, they must be part of a `$alert` section.

## mod_1wire

- `Randomize` renamed to `RandomizeProbes`
