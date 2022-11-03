# mod_owm

Publishes weather forecast from https://openweathermap.org

### Accepted global directives

* **APIkey=** API key you got from your Open Weather Map's account

## Section Meteo3H

Get 3 hours forecast from Open Weather Map API 2.5

### Accepted directives

* **Topic=** Topic to publish to.
* **Sample=** Delay between 2 queries in seconds (minimum is 600s)
* **City=** City targeted by our request
* **Units=** Value's units among `metric` (*default value*), `imperial` and `standard`
* **Lang=** Languages (*i.e. `fr` for French*)
* **Immediate** Launch the 1st sample at startup
* **Disabled** Start this section disabled

### Published topics

* `.../time` Timestamp of the forecast
* `.../temperature`
* `.../pressure`
* `.../humidity`
* `.../weather/description``
* `.../weather/code`
* `.../weather/acode` Adapted code
* `.../wind/speed`
* `.../wind/direction`

## Section MeteoDaily

Get daily forecast and today's sunrise/sunset from Open Weather Map API 2.5

### Accepted directives

* **Topic=** Topic to publish to.
* **Sample=** Delay between 2 queries in seconds (minimum is 600s)
* **City=** City targeted by our request
* **Units=** Value's units among `metric` (*default value*), `imperial` and `standard`
* **Lang=** Languages (*i.e. `fr` for French*)
* **Immediate** Launch the 1st sample at startup
* **Disabled** Start this section disabled

### Published topics

Per comming days :

* `.../*index*/time` Timestamp of the forecast
* `.../*index*/temperature/day`
* `.../*index*/temperature/night`
* `.../*index*/temperature/evening`
* `.../*index*/temperature/morning`
* `.../*index*/weather/description`
* `.../*index*/weather/code`
* `.../*index*/weather/acode`
* `.../*index*/pressure`
* `.../*index*/clouds`
* `.../*index*/snow`
* `.../*index*/wind/speed`
* `.../*index*/wind/direction`

and for today :

* `.../sunrise`
* `.../sunrise/GMT`
* `.../sunset`
* `.../sunset/GMT`
