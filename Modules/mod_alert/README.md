# mod_alert

Handles **alerts** and **notifications**.

## Difference between alerts and notifications

A message is sent for every **notification** received whereas Marcel keeps track of **alerts** state : 
a message is only sent when an alert is raised or cleared, but it sents only once when an alert is signaled many time without being cleared.
