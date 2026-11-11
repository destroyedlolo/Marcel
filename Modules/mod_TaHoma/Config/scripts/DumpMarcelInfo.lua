function DumpMarcelInfo()
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

        -- Access a field by a dedicated function
    	if s.inError then
	    	print("\tinError()", s:inError())
	    end
	end
end
