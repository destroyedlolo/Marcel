-- UPSBatCharge.lua
--
-- Rise an alert if my UPS run out of battery

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
