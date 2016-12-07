#!/usr/bin/lua

local LIMIT = 0.75	-- if a temperature is outside "moy +/- LIMIT", it is excluded
local moy = {}

print("Reading", arg[1] )

for l in io.lines( arg[1] ) do
	local Tref, Ts, diff, Tg = string.match(l:gsub(',','.'),'(.-)\t.-\t(.-)\t(.-)\t.-\t(.-)\t')
	Tref = tonumber(Tref)
	Ts = tonumber(Ts)
	diff = tonumber(diff)
	Tg = tonumber(Tg)

	if Tref < 80 and Ts < 80 and Tg < 80 then
		local delta = Tg - Ts
		if moy[delta] then
			moy[delta].nbre = moy[delta].nbre + 1
			moy[delta].sum = moy[delta].sum + diff
		else
			moy[delta] = { nbre = 1, sum = diff }
		end
	end
end

--[[
print "Calculated average :"

for delta, v in pairs( moy ) do
	print( delta, "n:" .. v.nbre, "s:" .. v.sum, "Moy:".. v.sum/v.nbre )
end
]]

print "2nd pass"
local nbre = 0
local exclus = 0
local f = assert( io.open( arg[2], 'w' ) )
f:write("Reference\tSonde\tDiff\tGrenier\n")

for l in io.lines( arg[1] ) do
	local Tref, Ts, diff, Tg = string.match(l:gsub(',','.'),'(.-)\t.-\t(.-)\t(.-)\t.-\t(.-)\t')
	Tref = tonumber(Tref)
	Ts = tonumber(Ts)
	diff = tonumber(diff)
	Tg = tonumber(Tg)

	if Tref < 80 and Ts < 80 and Tg < 80 then
		local delta = Tg - Ts
		nbre = nbre + 1

		if math.abs( diff - moy[delta].sum / moy[delta].nbre ) > LIMIT then
--			print("Ignoring :", diff, moy[delta].sum / moy[delta].nbre, diff - moy[delta].sum / moy[delta].nbre, math.abs( diff - moy[delta].sum / moy[delta].nbre ) )
			exclus = exclus + 1
		else
			f:write( string.format("%f\t%f\t%f\t%f\n", Tref, Ts, diff, Tg) )
		end
	end
end

f:close()

print( nbre .. " lines", exclus .. " excluded", exclus*100/nbre ..'%' )
