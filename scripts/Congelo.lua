-- Congelo.lua
--
-- This function sends an alert if the freezer
-- temperature goes to high.
--
-- Marcel's directives :
-- *DPD=Température Congélateur
-- Topic=maison/Temperature/Congelateur
-- Func=VerifCongelo

function VerifCongelo( topic, val )
        val = tonumber(val)
        if val > -15 and not alertcongelo then
                alertcongelo = true
                Marcel.SendNamedMessage("ES", "Alerte Temperature congélateur", "La temperature est trop haute : ".. val)
        elseif val <= -15 and alertcongelo then
                alertcongelo = false
                Marcel.SendNamedMessage("ES", "Temperature congélateur corrigée", "La temperature est correcte : ".. val)
        end
end

