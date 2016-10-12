-- SituationPorteGarage.lua
--
-- Translate gate's code to human readable condition
-- Notify as well

function SituationPorteGarage( topic, val )
	local res = 'Verrouillee'
	val = tonumber( val )

	if val==3 then
		res = 'Ouverte'
	elseif val==1 then
		res = 'Fermee'
	end

	if ans_PorteGarage ~= res then
		ans_PorteGarage = res

		Marcel.MQTTPublish( topic:sub(1, -6), res, true )
		Marcel.SendNamedMessage('i', "Porte du garage", res)
	end
end
