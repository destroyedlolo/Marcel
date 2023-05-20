mod_OnOff
===

This module provides an easy way to enable/disable a section through the topic `<ClientID>/OnOff/<Section identifier>`. 
If the payload is equal to `0`, `off` or `disable`, the section is ... disabled. Any other value enables it.

Example :

`"off" -> Marcel/OnOff/About`
will disable a section named **about** on an instance named **Marcel**.
