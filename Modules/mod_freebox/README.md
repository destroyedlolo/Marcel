mod_freebox
====

Publish **FreeboxOS** powered figures (French Internet Service Provider, group Iliad)

**mod_freeboxV6** module can publish old Freebox v4 and V5.

# Prerequisite

some configurations are needed on your Freebox to give access to Marcel

## Request for application token

Issue following curl command (obviously, you can put anything you want, especially regarding the device name).

```
curl -d '{"app_id": "Marcel", "app_name":  "Marcel", "app_version": "v8", "device_name": "bPI"}' -X POST http://mafreebox.freebox.fr/api/v4/login/authorize/
```
Response looks like :
```
{"success":true,"result":{"app_token":"iNCwLP9Ff6CO3BFX4RDRrDZkMNkd1SEshxDlqGflxlqoWllzIk4mK9PxFR\/jWtRV","track_id":1}
```

Provided app_token has to be put in Marcel's configuration and "Marcel" needs to be accepted on your Freebox screen. Then, you can fine tune it's rights from FreeboxOS's console (*Acces management/Applications*).


