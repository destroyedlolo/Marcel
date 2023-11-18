-- PublishMarcelStatus
-- publishes the real-time status of each defined section

function PublishMarcelStatus(id, topic)
	topic = topic .. '/response'	-- build response topic

	local res = ''
	for s in Marcel.Sections()
	do
		res = res .. s:getName() ..'\t'.. s:getKind() ..'\t'..
			(s:isEnabled() and '1' or '0')
		if s.inError then
			res = res ..'\t'.. (s:inError() and '1' or '0')
		end
		res = res .. '\n'
	end

	if MARCEL_VERBOSE then	-- log the result
		Marcel.Log('I', "Marcel's status \n" ..
			"Name\tType\tEnable\tinError\n" ..
			"------\n" .. res)
	end

	Marcel.MQTTPublish(topic, res)
end
