-- Function called my mod_test
-- This is an example of Lua callback which is fully section kind dependant
--
-- Here, we got the "dummy" field value and display it in this script
-- or in the section code itself.
--

function func_Test( val )
	if val % 3 == 0 then
		print("Lua script, dummy :", val)
		return false	-- Don't run remaining section code
	end

	return true	-- continue with the section implementation
end
