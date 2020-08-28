-- DetermineVacances
-- 
-- This function determines and then publish if we are holiday or not
-- (not used anymore)
--

json = require("dkjson")

function _DetermineVacances( info, topic, offset )
-- info : received information, NIL in case of error
-- topic : topic to publish to
-- offset : offset in day to apply to current date in case
-- 		we have to determine weekday (if info is NIL)

	local r

	if info then	-- Get from the webservice
		local res = json.decode(info)
		if res then	-- Valid response
			r = "Vacances"
			if res.weekend == "False" and res.holiday == "False" then
				r = "Travail"
			end
		end
	end

		-- safety checks
	local t = os.time() + offset * 86400 -- 24 * 60 * 60
	t = os.date('*t', t)['wday']
	if t == 1 or t == 7 then	-- Sat or Sun -> force Holidays
		r = "Vacances"
	elseif not r then	-- Workday by default
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
