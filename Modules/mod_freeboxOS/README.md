mod_freeboxOS
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

# Marcel's configuration

### Accepted global directives
none

## Section Freebox

### Accepted directives
* **URL=**  Where too contact the Freebox
* **app_token** application token
* **Topic=** root of topics
* **Sample=** Number of seconds between samples, in seconds
* **Keep** Do not die in case of error, sleep until next run [optional]
* **Immediate** Execute at startup (or when the section is enabled) then wait for *sample* seconds [optional]
* **Disabled** Section is disabled at startup [optional]

## Example

An example is provided in `Config` sub directory of the current one
