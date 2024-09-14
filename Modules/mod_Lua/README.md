mod_Lua
====

Add Lua's plugins to Marcel, allowing user functions

## Configuration

### Accepted global directives
* **UserFuncScript=** - script to be executed to define user functions.

> [!WARNING]
> * as of v8.6.1, all **UserFuncScript** are executed in sequence, after Marcel's own initialisation.
> * callback functions are launched in the same Lua's state. Consequently, all objects are available to others but functions **have** to be as fast as possible (as they will block other one until completion)
> * scripts are fully executed : it's a convenient way to do some initialisation

# Exposed objects to Lua's side

**Marcel** exposes some objects to user's scripts.<br>
Here is the list of ones exposed by **mod_Lua** itself : other modules may expose additional objects, take a look at their own documentation.

## Exposed variables

  * **MARCEL_SCRIPT** - name of the script being executed.
  * **MARCEL_SCRIPT_DIR** - directory holding the script.
  * **MARCEL_VERBOSE** - Marcel is running in verbose mode.
  * **MARCEL_DEBUG** - Marcel is running in debug mode.

> [!WARNING]
> As sharing the same Lua state, **MARCEL_SCRIPT** and **MARCEL_SCRIPT_DIR** are
> only accurate when scripts are loaded. Afterward, for example when functions are called, values are the ones of the last script.

## Exposed objects

### Marcel

The main interface to Marcel internals. The following functions are exposed :

  * **Marcel.Copyright()** - Returns Marcel's copyright string
  * **Marcel.Version()** - Returns Marcel's version
  * **Marcel.ClientID()** - Returns Marcel's MQTT client ID
  * **Marcel.Hostname()** - Returns the host's name
  * **Marcel.Log( *level*, *message* )** - Log a message with the provided level
  * **Marcel.MQTTPublish( *topic*, *payload* [, *retain* ] )** - Publish to MQTT

### Sections

#### Marcel's methods related to sections

  * **Marcel.Sections()** - Return an iterator to sections
  * **Marcel.FindSection( *name* )** - Find a section by its name. Returns **nil** if not found

#### Generic Interface to Marcel's section.

  * **getName()** and **getUID()** return section name.
  * **getKind()** - return section's kind
  * **isEnabled()** - return if the section is enabled (*disable* flag)
  * **getCustomFigures()** - return a table containing the section's specifics figures

Simple code example : lists sections, namedNotification and exposes related figures
```Lua
for s in Marcel.Sections()
do 
	print('"' .. s:getName() .. '"', s:getKind(), s:isEnabled() )

	local cust = s:getCustomFigures()
	if cust then
		for k,v in pairs(cust) do
			if type(v) == 'table' then
				for _, vv in pairs(v)
				do
					print('\t\t', vv)
				end
			else
				print('\t', k .. ':', v)
			end
		end
	end

        -- Access a field by a dedicated function
    if s.inError then
	    print("\tinError()", s:inError())
    end
end

print ""

if mod_alert then	-- Only if mod_alert is loaded
	print ""
	print "Defined Named notifications"
	print "---------------------------"

	for n in mod_alert.NamedNotifications()
	do
		print('"' .. n:getName() .. '"', n:isEnabled() )
	end
end

print ""
```
