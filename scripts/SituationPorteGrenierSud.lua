-- SituationPorteGrenierSud.lua
--
-- Translate gate's code to human readable condition
-- Notify as well

function SituationPorteGrenierSud( topic, val )
    local res = 'Ouverte'
    val = tonumber( val )

    if val==0 then
        res = 'Fermee'
    end

    if ans_PorteGS ~= res then
        ans_PorteGS = res

        Marcel.MQTTPublish( topic:sub(1, -7), res, true )
        Marcel.SendNamedMessage('i', "Porte du grenier sud", res)
    end
end
