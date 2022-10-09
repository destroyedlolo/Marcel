-- PublishMarcelInfo
--
-- Publish Marcel information

function PublishMarcelInfo()
	Marcel.Log('I', "Marcel's version ".. Marcel.Version() )
	Marcel.Log('I', Marcel.Copyright() )

-- only if mod_alert is loaded
--	Marcel.SendAlertsCounter()
end
