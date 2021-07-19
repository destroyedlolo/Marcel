-- FileNotFound.lua
--
-- This function is called when an FFV's file can't be opened
-- (i.e : when DS2482-800 is hanging and no 1-wire probe is exposed)

function FileNotFound( id, topic, errmsg )
	print("Error when accessing", file, "for", id)
end
