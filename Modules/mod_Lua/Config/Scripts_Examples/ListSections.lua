-- This scripts shows how to enumerate sections
--

print ""
print "Defined sections"
print "----------------"

for s in Marcel.Sections()
do 
	print('"' .. s:getName() .. '"', s:getKind(), s:isEnabled() )

	local cust = s:getCustomFigures()
	if cust then
		for k,v in pairs(cust) do
			if type(v) == 'table' then
				for _, vv in pairs(v)
				do
					print('\t\t', vv)
				end
			else
				print('\t', k .. ':', v)
			end
		end
	end
end


print ""
