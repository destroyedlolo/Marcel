-- Correct Chamber against attic temperature
function MemoriseTempGrNord( id, topic, val )
	GrN = val;
	return true
end

function CorrectTempJoris( id, topic, val )
-- Origin : -1.0064188535 (already compensated by the offset)
-- gradient : 0.1306052176
-- result : Probe - (( GrN - probe ) * gradient + org )
-- 			Probe -( GrN - probe ) * gradient - org

	if not GrN then
		return true	-- Submit the non corrected value
	else
		local v = val - (GrN - val)*0.1306052176 + 1.0064188535
print(GrN - val, val, v)
		Marcel.MQTTPublish( topic, v )
		return false
	end
end
