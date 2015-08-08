-- Production.lua
--
-- This function rises an alert if we are not producing 
-- electricity last 24 hours.
--
-- Marcel's directives :
-- *DPD=Production photovoltaïque
-- Topic=TeleInfo/Consommation/values/PAPP
-- Func=CheckProduction
--

local lastval
local ans

function CheckProduction( topic, val )
t = val
if ans then
	val = ans - val
end
ans = t
print(val)

	if val ~= 0 then
		lastval = os.time()
		Marcel.ClearAlert("Photovoltaïque")
	else
		if lastval then
print('diff', os.difftime( os.time(), lastval ))
			if os.difftime( os.time(), lastval ) > --[[ 24 * 60 * 60 ]] 5 then
				Marcel.RiseAlert("Photovoltaïque", "Pas de production depuis 24h\nVérifiez l'onduleur et le disjoncteur.")
			end
		end
	end
end
