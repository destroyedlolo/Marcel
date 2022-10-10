-- mod_sh31 example
-- Displays temperature and humidity values
--
-- It's only an example, real function should validate values
-- and returns 'true' if data have to be published and false otherwise

function DisplayTH( section_name, temperature, humidity )
	print("t:" .. temperature, "h:" .. humidity);

	return true;
end
