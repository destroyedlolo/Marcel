-- Validate the value returned by a probe

function ValTProbe( id, raw, compensated )
	print("Received value for '".. id .."'", "raw value :" .. raw, "compensated :" .. compensated)
	return true	-- Publish value
end
