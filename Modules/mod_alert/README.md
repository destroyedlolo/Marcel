# mod_alert

Handles **alerts** and **notifications**.

## Difference between alerts and notifications

A message is sent for every **notification** received whereas Marcel keeps track of **alerts** state : 
a message is only sent when an alert is raised or cleared, but it sent only once when an alert is signaled many time without being cleared.

## Directives accepted by all kind of sections handled by mod_alert

### Actions directives
* **RESTUrl=** call a REST webservice
* **OSCmd=** issue an OS command

In each command line, following replacement is done :
* `%t%` is replaced by the title
* `%m%` is replaced by the message

Example :
`RESTUrl=https://wirepusher.com/send?id=YoUrAPIKeY&title=Test:%t%&message=%m%&type=Test`

### Other

* **Disabled** : This section starts disabled (see *mod_OnOff*)

## Alerts

A message is sent only when an alert is **set** or **cleared**.

A typical use case is a regular monitoring (*let say regarding a fridge temperature, every 5 minutes*) reporting an error condition (*temperature too high*). 
- If communicating through notifications, a message will be sent at every sample, *so every 5 minutes*. 
- Using **alerts**, a message will be sent once when the temperature goes outside range, the alerts' counter will be increased, and another one when the temperature is corrected : alerts avoid being spammed by the same error condition.

### Accepted global directives

* **AlertsCounterTopic=** if set, send number of alerts every time an alert is raised or cleared.

Example :
`AlertsCounterTopic=%ClientID%/AlertsCounter`

### Additional topics

Following topics are used as well when alerting :

- When an alert is raised, a message to `%MarcelID%/Error` is published
- When cleared, `%MarcelID%/Log/Corrected` is published

### Alert sections

Notez-bien : there is no segregation between alerts created by $alert or \*RaiseAlert= , they are handled the same way internally (in other words, an $alert can be cleared by \*CorrectAlert= .

#### $alert

**$alert** is supported mostly for compatibility purposes with some of my ancient tools. It is suggested to use **\*RaiseAlert=** instead, which is more versatile and powerful. In the other hand, it can be used as *default alerting channel*.

**$alert** messages are received from `Alert/#` topic where the topic name's trailing part determines the *title*, the payload being the message. 

If the first character of the payload is a '**S**' or '**s**', the alert is raised, any other character means the alert is over.
- With a '**S**' both `RESTUrl=` and `OSCommand=` are triggered called.
- With a '**s**' only `OSCommand=` is called.

Only one can exist and is impersonated by a "*$alert*" section (if needed to be disabled by **OnOff** module). 

#### \*RaiseAlert= and \*CorrectAlert=

Several \*RaiseAlert= and \*CorrectAlert= sections could coexist : it's allowing different actions to trigger, but again, there is no segregation between alerts, every \*CorrectAlert= can clear an alert raised by any \*RaiseAlert= or even $alert.

##### Directives accepted by both \*RaiseAlert= and \*CorrectAlert=

- **Topic=** directive indicates which is the topic to listen to.

##### Directives accepted only by \*RaiseAlert=

- **Quiet** directive indicates it's a **special** section without topic and
action commands defined. This section is useful to raise an alert (i.e. while loading a backup at restart) without sending messages.

## Notifications

A message is sent for every **notification** received. So, if not correctly configured upstream, you may be spammed by zillion of message. Consequently, **notifications** aim is to report transitions' changes or punctual events.

### Notification sections

#### $unnamedNotification

**Unnamed Notification** is supported mostly for compatibility purposes with some of my ancient tools. It is suggested to use **[Named Notification](###$namednotification=)** instead which are more versatile and powerful. In the other hand, it can be used as *default notification channel*.

**Unnamed Notification** messages are received from `Notification/#` topic where the topic name's trailing part determines the *title*, the payload being the message. Only one can exist and is impersonated by a "*$unnamedNotification*" section (if needed to be disabled by **OnOff** module). 

If the payload starts with a '**S**', both `OSCmd=` and `RESTUrl=` are used ; Without 'S', only `OSCmd=` is triggered.

Example :
- topic : `Notification/My Example`
- payload : `SI want to tell you`

will send "*I want to tell you*" on both `OSCmd=` and `RESTUrl=` channels with the title "*My Example*".

### $namedNotification=

**Named notification** allows a more flexible in notification mechanism as each and every notification can communicate with a different channel. As example, I'm using [wirepusher](https://wirepusher.com/) to send notifications to my phone and each named notification targets a different message "type" that can be segregated at wirepusher's side.

Unlike unnamed notification, both `OSCmd=` and `RESTUrl=` are triggered if present, whatever the payload's content.

**Named notification** are received from `nNotification/<names>/<title>` topic where the argument is the list of notification to be considered.

Example : 
- topic `nNotification/abc/my title`
- payload : `Seems ok`

will send "*Seems ok*" to notifications **a**, **b** and **c** with "*my title*" as ... title.

Limitations :
- `$namedNotification=` argument is the section name and can be only 1 character long.
- `$namedNotification=` are not considered as sections and consequently can't be disabled by **OnOff** module (yet ?)

## Objects exposed to Lua
### Exposed functions

#### Legacy reporting
- **Marcel.RiseAlert( *AlertID*, *Message to send* )** - Raise an alert as it was received on `Alert/#` topic. Only `OSCmd=` is triggered.
- **Marcel.RiseAlertREST( *AlertID*, *Message to send* )** - Raise an alert as it was received on `Alert/#` topic. Both `OSCmd=` and `RESTUrl=` are triggered.
- **Marcel.ClearAlert( *AlertID*, *Message to send* )** - Clear an alert as it was received on `Alert/#` topic. Both `OSCmd=` and `RESTUrl=` are triggered.

- **Marcel.SendNotification( *AlertID*, *Message to send* )** - Send a notification as if was received on `Notification/#` topic. Only `OSCmd=` is triggered.
- **Marcel.SendNotificationREST( *AlertID*, *Message to send* )** - Send a notification as if was received on `Notification/#` topic. Both `OSCmd=` and `RESTUrl=` are triggered.

#### Named reporting

- **Marcel.RiseAlert( *AlertID*, *Message to send*, *name* )** - Raise an alert as it was received on *name* `\*RaiseAlert=` topic.
- **Marcel.ClearAlert( *AlertID*, *Message to send*, *name* )** - Clear an alert as it was received on *name* `\*CorrectAlert=` topic.
- **Marcel.SendNamedNotification( *Notifications names*, *AlertID*, *Message to send* )** - Send notifications as per provided names

#### Misc
- **Marcel.SendAlertsCounter()** - Send alert counter if `AlertsCounterTopic=` is defined
- **Marcel.ListAlert()** - Generate a list of all alerts' name.

Notez-bien : ListAlert() is pushing a string per alert, which is not the best way to handle a large amount (but we are not expecting zillion alerts at a time, do we ?). The reason not to use an iterator is it would need to lock alerts list during iterating. It may lead to alert in race condition. 
Have a look in `Config/scripts` directory for an example how to save/restore alerts list.
