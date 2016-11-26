-- Correct my daughter bedroom temperature

function MemoriseTempRef(id, topic, val, compensated )
	TRef=compensated
	return true;
end

function MemoriseTempGSud(id, topic, val, compensated )
	TGSud=val	-- as statistics are generated using raw value
	return true;
end

function CorrectChOceane(id, topic, val, compensated )
-- Origin : -0.668338296
-- gradient : 0.1138899491
-- result : Probe - (( GrN - probe ) * gradient + org )
-- 			Probe -( GrN - probe ) * gradient - org

	if not TGSud or not TRef then
		return true
	else
		local v = val - (TGSud - val)*0.1138899491 + 0.668338296
		Marcel.MQTTPublish( topic, v )
		print(TGSud - val, v-TRef, compensated-TRef)
	end
end
