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

	local r = "Vacances"

	if info then	-- Get from the webservice
		local res = json.decode(info)

		if res.weekend == "False" or res.holiday == "False" then
			r = "Travail"
		end
	else
		r = "Travail"
	end

		-- check if it wasn't bulshit
	local t = os.time() + offset * 86400 -- 24 * 60 * 60
	t = os.date('*t', t)['wday']
	if t == 1 or t == 7 then
		r = "Vacances"
	else
		r = "Travail"
	end

	Marcel.MQTTPublish( topic, r, true )
end

function DetermineVacancesAujourdHui( info )
	_DetermineVacances( info, 'Majordome/Mode/AujourdHui', 0 )
end

function DetermineVacancesDemain( info )
	_DetermineVacances( info, 'Majordome/Mode/Demain', 1 )
end


