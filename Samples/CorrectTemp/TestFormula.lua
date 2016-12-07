#!/usr/bin/lua

	-- Formula constant
local GRADIAN = -0,73517859539305
local ORDONNE = 0,023763691320168
local MOYEN = -0,748850328384412

function formula(Ts, Tg)
	return Ts - (Tg - Ts)*GRADIAN - ORDONNE
end

	-- reading file
print("Reading", arg[1] )
print("Writing", arg[2] )

local f = assert( io.open( arg[2], 'w' ) )
f:write("Tg-Ts\tCalc-Tr\tComp-Tr\n")

for l in io.lines( arg[1] ) do
	local Tref, Ts, diff, Tg = string.match(l:gsub(',','.'),'(.-)\t.-\t(.-)\t(.-)\t.-\t(.-)\t')
	Tref = tonumber(Tref)
	Ts = tonumber(Ts)
	diff = tonumber(diff)
	Tg = tonumber(Tg)

	if Tref < 80 and Ts < 80 and Tg < 80 then
		f:write( Tg-Ts ..'\t'.. formula(Ts,Tg)-Tref ..'\t'.. Ts-MOYEN-Tref ..'\n' )
	end
end

f:close()

