#!/usr/bin/lua

	-- Formula constants
if true then
	print "with all data"
	GRADIAN = 0.0459863012
	ORDONNE = -0.7313800085
	MOYEN = -0.7589104392

else
	print "with selected data"
	GRADIAN = 0.1011153652
	ORDONNE = -0.6457722383
	MOYEN = -0.707761225
end

function formula(Ts, Tg)
	return Ts - (Tg - Ts)*GRADIAN - ORDONNE
end

	-- reading file
print("Reading", arg[1] )
print("Writing", arg[2] )

local f = assert( io.open( arg[2], 'w' ) )
f:write("Tg-Ts\tTs-Tr\tCalc-Tr\tComp-Tr\n")

for l in io.lines( arg[1] ) do
	local Tref, Ts, diff, Tg = string.match(l:gsub(',','.'),'(.-)\t.-\t(.-)\t(.-)\t.-\t(.-)\t')
	Tref = tonumber(Tref)
	Ts = tonumber(Ts)
	diff = tonumber(diff)
	Tg = tonumber(Tg)

	if Tref < 80 and Ts < 80 and Tg < 80 then
		local Tf = formula(Ts,Tg)-Tref
		local Tc = Ts-MOYEN-Tref 
		f:write( Tg-Ts ..'\t'.. Ts-Tref  ..'\t'.. Tf ..'\t'.. Tc .. '\t' .. (Tg-Ts)*GRADIAN .. '\n' )
	end
end

f:close()

