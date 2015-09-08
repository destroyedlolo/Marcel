-- PublishLoad
-- 
-- This function publish host load
--

function PublishLoad( section_name )
	print ("bip : " .. section_name)

	Marcel.MQTTPublish("Test", section_name)
end

