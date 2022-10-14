
-- This function is called when an FFV's file can't be opened
-- (i.e : when DS2482-800 is hanging and no 1-wire probe is exposed)

function NoProbe( id, errmsg )
        print("Error when accessing file for '".. id .."'", errmsg)
end
