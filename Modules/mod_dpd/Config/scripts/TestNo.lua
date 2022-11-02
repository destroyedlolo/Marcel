-- Test callback function
-- 
-- Return false if val == "No", true otherwise
--

function TestNo( section_name, topic, val )
	print("Test", section_name, val)

	if val == "No" then
		return false
	else
		return true
	end
end
