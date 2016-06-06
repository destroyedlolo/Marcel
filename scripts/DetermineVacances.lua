-- DetermineVacances
-- 
-- This function determines and then publish if we are holiday or not
--

json = require("dkjson")

function _DetermineVacances( info, topic )
	local res = json.decode(info)

	if res.weekend ~= "False" or res.holiday ~= "False" then
		Marcel.MQTTPublish( topic, "Vacances", true)
	else
		Marcel.MQTTPublish( topic, "Travail", true )
	end
end

function DetermineVacancesAujourdHui( info )
	_DetermineVacances( info, 'Majordome/Mode/AujourdHui' )
end

function DetermineVacancesDemain( info )
	_DetermineVacances( info, 'Majordome/Mode/Demain' )
end


