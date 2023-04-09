# mod_alert

Handles **alerts** and **notifications**.

## Difference between alerts and notifications

A message is sent for every **notification** received whereas Marcel keeps track of **alerts** state : 
a message is only sent when an alert is raised or cleared, but it sents only once when an alert is signaled many time without being cleared.

## kinds

### $unnamedNotification

**Unnamed Notification** is supported mostly for compatibility purposes with some of my ancient tools. It is suggested to use **Named Notification** instead are more versatile and powerful. In the other hand, it can be used as *default notification channel*.

**Unnamed Notification** message are received from `Notification/#` topic where the topic name's trailing part determines the *title*, the payload being the message. Only one can exist and is impersonated by a "*$unnamedNotification*" section (if needed to be disabled by **OnOff** module).

if the payload starts with a '**S**', both `OSCmd` and `RESTUrl` are used ; Without 'S', only `OSCmd` is triggered.

### $namedNotification=


**Named notification** allows a more flexible in notification mechanism as each and every notification can communicate with a different channel. As example, I'm using [wirepusher](https://wirepusher.com/) to send notifications to my phone and each named notification targets a different message "type" that can be segregated at wirepusher's side.

Unlike unnamed notification, both `OSCmd` and `RESTUrl` are triggered if present, whatever the payload's content.

**Named notification** are received from `nNotification/<names>/<title>` topic where the argument is the list of notification to be considered. So `nNotification/abc/my title` will trigger notifications **a**, **b** and **c** with "*my title*" as ... title.

Limitations :
- `$namedNotification=` argument is the section name and can be only 1 character long.
- `$namedNotification=` are not considered as sections and consequently can't be disabled (yet) by **OnOff** module.

## Directives accepted by all kind of sections

* **RESTUrl=** call a REST webservice
* **OSCmd=** issue an OS command

In each command line, following replacement is done :
* `%t%` is replaced by the title
* `%m%` is replaced by the message

Example :
`RESTUrl=https://wirepusher.com/send?id=YoUrAPIKeY&title=Test:%t%&message=%m%&type=Test`
