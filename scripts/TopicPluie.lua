-- TopicPluie

--
-- This function enables/disables "Pluie" topic
--
function ActiveTopicPluie( section_name, val )
	Marcel.MQTTPublish( Marcel.ClientID()..'/OnOff/Pluie', val )
end
