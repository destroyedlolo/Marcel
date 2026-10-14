
-- Exemple to trace incomming message

function ValCommand( id, topic, val )
	print("Received value for '".. id .."'", "value : " .. val)

	if val == "0" then
		return false;	-- Don't do anything
	elseif val == "1" then
		return true;	-- Ok, do it
	else
		return "what else"
	end
end
