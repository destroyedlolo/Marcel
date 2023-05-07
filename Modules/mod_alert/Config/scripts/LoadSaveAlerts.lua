-- Load / Save current alert list


function SaveAlert( file )
	local t = table.pack( Marcel.ListAlert() )

	local f,err = io.open(file,'w')
	if not f then
		Marcel.Log('E', "Can't open \"".. file .."\" : " .. err)
		return
	end

	for _,v in pairs(t) do
		f:write(v,'\n')
	end

	f:close()
end

function LoadAlert( file )
	local f,err = io.open(file,'r')
	if not f then
		Marcel.Log('E', "Can't open \"".. file .."\" : " .. err)
		return
	end

	for v in f:lines() do
		Marcel.RiseAlert(v, '', 'silent')
	end
end

-- Some code example to save and then load alerts

-- Test saving facility

--[[
	-- Feed with random alerts
Marcel.RiseAlert("Eiz", "toto", "alerting")
Marcel.RiseAlert("1", "toto", "alerting")
Marcel.RiseAlert("al2", "toto", "alerting")
Marcel.RiseAlert("3", "toto", "alerting")
Marcel.RiseAlert("al4", "toto", "alerting")
print(Marcel.ListAlert())

	-- saving
SaveAlert("/tmp/alerts")

	-- cleaning
Marcel.ClearAlert("3", "rien", "correcting")
print(Marcel.ListAlert())
]]

-- Test restoring

--[[
LoadAlert("/tmp/alerts")
print(Marcel.ListAlert())
]]
