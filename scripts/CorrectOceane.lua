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
-- result : Probe - (( GrN - probe ) * gradient + org )
-- 			Probe -( GrN - probe ) * gradient - org
	GRADIAN = 0.368026227935636
	ORDONNE = -0.521081761772103

	if not TGSud or not TRef then
		return true
	else
		if TGSud > 80 or TRef > 80 or val > 80 then	-- a probe sent bulshit
			return false
		end

		local v = val - (TGSud - val)*GRADIAN - ORDONNE
		Marcel.MQTTPublish( topic, v )
--		Marcel.MQTTPublish( topic..'/cmp', TGSud-val ..',' .. v-TRef ..',' .. compensated-TRef ..',' .. v2-TRef )
		Marcel.MQTTPublish( topic..'/cmp', TGSud-val ..',' .. v-TRef ..',' .. compensated-TRef ..',' .. val )
	end
end
