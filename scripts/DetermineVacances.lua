-- DetermineVacances
-- 
-- This function determines and then publish if we are holiday or not
--

json = require("dkjson")

function _DetermineVacances( info, topic, offset )
-- info : received information, NIL in case of error
-- topic : topic to publish to
-- offset : offset in day to apply to current date in case
-- 		we have to determine weekday (if info is NIL)

	if info then
		local res = json.decode(info)

		if res.weekend ~= "False" or res.holiday ~= "False" then
			Marcel.MQTTPublish( topic, "Vacances", true)
		else
			Marcel.MQTTPublish( topic, "Travail", true )
		end
	else
		local t = os.time() + offset * 86400 -- 24 * 60 * 60
		t = os.date('*t', t)['wday']
		if t == 1 or t == 7 then
			Marcel.MQTTPublish( topic, "Vacances", true)
		else
			Marcel.MQTTPublish( topic, "Travail", true )
		end
	end
end

function DetermineVacancesAujourdHui( info )
	_DetermineVacances( info, 'Majordome/Mode/AujourdHui', 0 )
end

function DetermineVacancesDemain( info )
	_DetermineVacances( info, 'Majordome/Mode/Demain', 1 )
end


