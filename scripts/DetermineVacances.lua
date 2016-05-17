-- DetermineVacances
-- 
-- This function determines and then publish if we are holiday or not
--

require "tostring"

function DetermineVacances( info )
	local res = json.decode(info)
	print( universal_tostring(res) )

	if res.weekend ~= "False" or res.holiday ~= "False" then
		Marcel.MQTTPublish( "Profile/Mode", "Vacances", true)
	else
		Marcel.MQTTPublish( "Profile/Mode", "Travail", true )
	end
end

