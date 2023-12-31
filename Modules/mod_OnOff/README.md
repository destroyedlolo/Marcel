mod_OnOff
===

This module provides an easy way to enable/disable a section or a named notification.

# Section

Sections are controled through the topic `%ClientID%/OnOff/<Section identifier>`. 
If the payload is equal to `0`, `off` or `disable`, the section is ... disabled. Any other value enables it.

Example :

`"off" -> Marcel/OnOff/About`
will disable a section named **about** on an instance named **Marcel**.

# Named Notification (if mod_alert is loaded)

Named Notification are controled through the topic `%ClientID%/NamedNotifOnOff/<Named Notification identifier>`. 
If the payload is equal to `0`, `off` or `disable`, the named notification is ... disabled. Any other value enables it.

Example :

`"off" -> Marcel/NamedNotifOnOff/p`
will disable notification named **p** on an instance named **Marcel**.
