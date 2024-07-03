mod_owm
===

Publishes weather forecasts from https://openweathermap.org

### Accepted global directives

* **APIkey=** API key you got from your Open Weather Map's account

## Section Meteo3H

Get a 3-hour forecast from the Open Weather Map API 2.5

### Accepted directives

* **Topic=** Topic to publish to.
* **Sample=** Delay between 2 queries in seconds (minimum is 600s)
* **City=** City targeted by our request
* **Units=** Value's units among `metric` (*default value*), `imperial` and `standard`
* **Lang=** Languages (*i.e. `fr` for French*)
* **Immediate** Launch the first sample at startup
* **Disabled** Start this section disabled
*  **DoNotSimulate** disables this section when running in simulation mode.

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

Get daily forecast and today's sunrise/sunset from the Open Weather Map API 2.5

### Accepted directives

* **Topic=** Topic to publish to.
* **Sample=** Delay between 2 queries in seconds (minimum is 600s)
* **City=** City targeted by our request
* **Units=** Value's units among `metric` (*default value*), `imperial` and `standard`
* **Lang=** Languages (*i.e. `fr` for French*)
* **Immediate** Launch the 1st sample at startup
* **Disabled** Start this section disabled

### Published topics

Per the coming days :

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

## Error condition

An error condition is associated to each section, individually. It is raised if a technical issue prevents to read data and is cleared as soon as an attempt succeeds.

Error condition is exposed to Lua by the **MeteoDaily:inError()** and **Meteo3H:inError()** methods.
