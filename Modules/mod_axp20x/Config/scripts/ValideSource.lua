-- mod_axp20x example
-- Reject data if voltage is 0

function ValideSource( section_name, figure, voltage, current )
	if voltage == 0 then
		return false
	end

	return true
end
