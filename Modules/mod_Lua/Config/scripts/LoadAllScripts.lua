-- This script load in memory all scripts found in this directory
-- They are mainly expected to create users functions

require'lfs'
for file in lfs.dir(MARCEL_SCRIPT_DIR) do
    if lfs.attributes(MARCEL_SCRIPT_DIR .. '/' .. file,"mode") == "file" and 
		file ~= MARCEL_SCRIPT and
		file ~= "README.md" and
		file:sub(1, 1) ~= '.'
	then
		if MARCEL_DEBUG then
			print("*Lua* Running ", file)
		end
		dofile(MARCEL_SCRIPT_DIR .. '/' .. file)
	end
end

