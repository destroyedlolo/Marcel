-- Functions.lua
--
-- This file contains some example for user defined function
-- that are called by when receiving a DPD value

function UPSBatCharge( topic, val )
	if val < 20 then
		Marcel.RiseAlert("Battery charge is critical", val)
	elseif val < 50 then
		Marcel.RiseAlert("Battery charge is low", val)
	else	-- Clear alerts potentially sent
		Marcel.ClearAlert("Battery charge is critical");
		Marcel.ClearAlert("Battery charge is low");
	end
end
