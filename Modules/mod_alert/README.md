# mod_alert

Handles **alerts** and **notifications**.

## Difference between alerts and notifications

A message is sent for every **notification** received whereas Marcel keeps track of **alerts** state : 
a message is only sent when an alert is raised or cleared, but it sents only once when an alert is signaled many time without being cleared.

## Directives accepted by all kind of sections

* **RESTUrl=** call a REST webservice
* **OSCmd=** issue an OS command

In each command line, following replacement is done :
* `%t%` is replaced by the title
* `%m%` is replaced by the message

Example :
`RESTUrl=https://wirepusher.com/send?id=YoUrAPIKeY&title=Test:%t%&message=%m%&type=Test`
