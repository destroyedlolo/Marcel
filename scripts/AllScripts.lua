-- This script load all other scripts
-- (only one UserFuncScript directive can present in the configuration file)

json = require("dkjson")

-- Note : the global variable MARCEL_SCRIPT_DIR
-- contains the directory of this script.
require ( MARCEL_SCRIPT_DIR .. "/Photovoltaics" )
require ( MARCEL_SCRIPT_DIR .. "/UPSBatCharge" )
require ( MARCEL_SCRIPT_DIR .. "/PublishLoad" )
require ( MARCEL_SCRIPT_DIR .. "/Congelo" )
require ( MARCEL_SCRIPT_DIR .. "/DetermineVacances" )
require ( MARCEL_SCRIPT_DIR .. "/SituationPorteCave" )
require ( MARCEL_SCRIPT_DIR .. "/SituationPorteGarage" )
require ( MARCEL_SCRIPT_DIR .. "/SituationPorteGrenierSud" )
require ( MARCEL_SCRIPT_DIR .. "/TopicPluie" )

if tonumber(Marcel.Version()) < 3.3 then
	print("*F* Marcel 3.3 or newer needed by Lua functions")
	os.exit( 20 )
end

