-- Publish Named Notifications real time status

function PublishNNStatus(id, topic)
	topic = topic .. '/response'	-- build response topic
	local res = ''

	for n in mod_alert.NamedNotifications()
	do
		res = res .. n:getName() ..'\t'.. (n:isEnabled() and '1' or '0')
		res = res .. '\n'
	end

	if MARCEL_VERBOSE then	-- log the result
		Marcel.Log('I', "Named Notifications status \n" ..
			"Name\tEnable\n" ..
			"------\n" .. res)
	end

	Marcel.MQTTPublish(topic, res)
end
