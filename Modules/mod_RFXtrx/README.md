# mod_RFXtrx

Controle [RFXCom](http://www.rfxcom.com/en_GB) devices.<br>
As of V8.0, only RTS Shutters are supported.
You can help to improve Marcel to support other devices by :
- provide a patch request to add missing devices in Marcel's code
- provide devices you want to be added

### Accepted global directives

- **RFXtrx_Port=** "serial" port where RFXcom is plugged on.<br>
I strongly suggest to use `by-id` or `by-path` mapping instead of `/dev/ttyUSB??` which depend on plugged devices.
Consequently, a device can't be associated to a ttyUSB port in a predicted way.

## Section RTSCmd

Control àn RTS shutter.

### Accepted directives

- **ID=** number identifying a device
- **Topic=** Topic to send control commands to

### Accepted commands

- **Stop** or **My** : stop a shutter in motion or ask it to move to *my* preset position.
- **Up** : open a shutter
- **Down** : close a shutter
- **Program** : associate a shutter with corresponding address (hardly untested)
