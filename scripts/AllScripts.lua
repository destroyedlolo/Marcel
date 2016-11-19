-- This script load all other scripts
-- (only one UserFuncScript directive can present in the configuration file)

json = require("dkjson")

if tonumber(Marcel.Version()) < 3.3 then
	print("*F* Marcel 3.3 or newer needed by Lua functions")
	os.exit( 20 )
end

-- Note : the global variable MARCEL_SCRIPT_DIR
-- contains the directory of this script.
-- In order to load modules, we need to modify Lua seach path, but unlike
-- modifying the system wide LUA_PATH (which is a security hole), we modify
-- our local setting.
package.path = MARCEL_SCRIPT_DIR .. "/?.lua;" .. package.path


require ( "Photovoltaics" )
require ( "UPSBatCharge" )
require ( "PublishLoad" )
require ( "Congelo" )
require ( "DetermineVacances" )
require ( "SituationPorteCave" )
require ( "SituationPorteGarage" )
require ( "SituationPorteGrenierSud" )
require ( "TopicPluie" )

require ( "CorrectTempJoris" )
