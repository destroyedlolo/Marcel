-- PublishLoad
-- 
-- This function publish host load
--

function PublishLoad( section_name )
	local f = io.open('/proc/loadavg','r')
	local r1, r5, r10 = f:read('*n'), f:read('*n'), f:read('*n')
	f:close()

	local topic = 'Load/' .. Marcel.Hostname() ..'/'

	Marcel.MQTTPublish( topic .. '1', r1)
	Marcel.MQTTPublish( topic .. '5', r5)
	Marcel.MQTTPublish( topic .. '10', r10)
end

