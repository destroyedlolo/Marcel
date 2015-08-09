-- Production.lua
--
-- This function rises an alert if we are not producing 
-- electricity last 24 hours.
--
-- Marcel's directives :
-- *DPD=Production photovoltaïque
-- Topic=TeleInfo/Production/values/PAPP
-- Func=CheckProduction
--

local lastval

function CheckProduction( topic, val )
	if val ~= 0 then
		lastval = os.time()
		Marcel.ClearAlert("Photovoltaïque")
	else
		if lastval then
			if os.difftime( os.time(), lastval ) > 24 * 60 * 60 then
				Marcel.RiseAlert("Photovoltaïque", "Pas de production depuis 24h\nVérifiez l'onduleur et le disjoncteur.")
			end
		end
	end
end
