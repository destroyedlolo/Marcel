require "scripts/Photovoltaics"
require "scripts/UPSBatCharge"
require "scripts/PublishLoad"
require "scripts/Congelo"

if tonumber(Marcel.Version()) < 3.3 then
	print("*F* Marcel 3.3 or newer needed by Lua functions")
	os.exit( 20 )
end
