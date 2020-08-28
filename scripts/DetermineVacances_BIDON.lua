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

	print("_DetermineVacances() BIDON")
end

function DetermineVacancesAujourdHui( info )
	_DetermineVacances( info, 'Majordome/Mode/AujourdHui', 0 )
end

function DetermineVacancesDemain( info )
	_DetermineVacances( info, 'Majordome/Mode/Demain', 1 )
end


