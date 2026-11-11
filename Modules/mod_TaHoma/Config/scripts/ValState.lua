
-- Exemple to trace incomming message

function ValState( id, state, val )
	print("Received value for '".. id .."'/'".. state .."'", "value : " .. val)

	return false
end
