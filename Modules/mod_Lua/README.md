# mod_Lua
Add Lua's plugins to Marcel, allowing user functions

## Configuration

### Accepted global directives
* **UserFuncScript=** - script to be executed to define user functions.

⚠️Notez-Bien⚠️ : 
  * only the last **UserFuncScript** is considered. If you want to load 
several scripts, provided **LoadAllScripts.lua** will load all script found in the same directory
  * callback functions are launched in the same Lua's state. Consequently, all objects are available 
to others but functions **have** to be as fast as possible (as they will block other one until completion)

# Exposed objects to Lua's side

**Marcel** exposes some objects to user scripts

### Exposed variables

  * **MARCEL_SCRIPT** - name of the script being executed (usually, always **LoadAllScripts.lua**, the only usage is internally to `LoadAllScripts`).
  * **MARCEL_SCRIPT_DIR** - directory holding the script.
  * **MARCEL_VERBOSE** - Marcel is running in verbose mode.
  * **MARCEL_DEBUG** - Marcel is running in debug mode.

### Exposed functions

  * **Marcel.Copyright()** - Returns Marcel's copyright string
  * **Marcel.Version()** - Returns Marcel's version
  * **Marcel.ClientID()** - Returns Marcel's MQTT client ID
  * **Marcel.Hostname()** - Returns host's name
  * **Marcel.Log( *level*, *message* )** - Log a message with the provided level
  * **Marcel.MQTTPublish( *topic*, *payload* [, *retain* ] )** - Publish to MQTT