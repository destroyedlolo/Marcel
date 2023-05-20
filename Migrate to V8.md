As V8 deeply broke syntax compatibility with previous version, here some notes took when I switched my own installation.

# Compilation

If facing some MQTT related linking error during compilation, updgrade **paho** library.

# Global settings 

- ` ConnectionLostIsFatal` - Doesn't exist anymore, as it's now the default behavior.
I gave up to implement automatic reconnect : it's far better to handle it externally within the system's startup mechanism.

# Modules' specifics

## mod_alert

- `$notification=` (and its alias `$alert=`)  are deprecated. They are replaced by `$namedNotification=` which share the same syntax and usage.<br>
Notez-bien : `$alert` (without trailing '=') declares an unnamed alert.
- `SMSUrl=` and `AlertCommand=` are deprecated.
- `OSCmd=` and `RESTUrl=` are not any more accepted as global directives. To  achieve the same behavior, they must be part of an `$alert` section.

## mod_1wire

- `Randomize` renamed to `RandomizeProbes`
