
-- Exemple to trace incomming message

function ValEvent( id, event_id, val )
	print("Received value for '".. id .."'/'".. event_id .."'", "value : " .. val)

	return false
end
