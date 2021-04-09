This file contains internal notes for the development or before addition in the main documentation.

Alarming
========

Topic : **Alarm/< *subject* >**
	- < *subject* > to identify the alert 

Payload : [S|E]error message
* **S** : start of the alert
* **E** : End of the alert
* error message : information to be send.

Lua user function
=================

Lua stores its own information in a context which is not thread safe.
I had 2 options :
1. to create a dedicated context per section (implemented as thread), but in this case, each and every callback lives its own life and can't share code or information with others
1. to use an unique context and lock it's access.

I choose for Marcel the 2nd way which is more "natural" for users, especially because these scripts are only made for fast data validation and not for full automation (have a look on [Majordome]( https://github.com/destroyedlolo/Majordome) for that). 

As a consequence, only one callback can run at a time. It means callbacks must be as fast as possible to avoid dead locks.

1-wire alarms
=============

basement door :
---------------

Probe : `/var/lib/owfs/mnt/12.16CCCC000000/`

Be notified when a PIO changes 

```
	echo 311 >  /var/lib/owfs/mnt/12.16CCCC000000/set_alarm
```
		3: Both ports
		1: latches
		1: high

Keep
====

Working for :

**Freebox**

**UPS**

**DPD**

Useless for :

**FFV**

**EVERY**

**REST**

**METEO**

**RTSCmd**

**OutFile**


Retained
========

Applicable to :

**FFV**

**LookForChanges**

**Freebox**

**Meteo3H** (ignored, always on)

**MeteoDaily** (ignored, always on)

**UPS**


This directive is ignored for other categories.
