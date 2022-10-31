-- Notify test function
--
-- Return false if file == "No", true otherwise
--

function TestNotif(section_name, filename, actions)
	if MARCEL_DEBUG then
		print("TestNotif", section_name, filename, actions);
	end

	if filename == "No" then
		return false
	else
		return true
	end
end
