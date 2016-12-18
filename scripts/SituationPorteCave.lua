-- SituationPorteCave.lua
--
-- Translate gate's code to human readable condition
-- Notify as well

function SituationPorteCave( topic, val )
    local res = 'Ouverte'
    val = tonumber( val )

    if val==0 then
        res = 'Fermee'
    end

    if ans_PorteCave ~= res then
        ans_PorteCave = res

        Marcel.MQTTPublish( topic:sub(1, -7), res, true )
        Marcel.SendNamedMessage('i', "Porte de la cave", res)
    end
end
