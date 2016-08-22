-- This script load all other scripts
-- (only one UserFuncScript directive can present in the configuration file)

json = require("dkjson")

require "scripts/Photovoltaics"
require "scripts/UPSBatCharge"
require "scripts/PublishLoad"
require "scripts/Congelo"
require "scripts/DetermineVacances"
require "scripts/SituationPorteCave"
require "scripts/TopicPluie"

if tonumber(Marcel.Version()) < 3.3 then
	print("*F* Marcel 3.3 or newer needed by Lua functions")
	os.exit( 20 )
end

