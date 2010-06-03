-- ============================================================================
-- A hash table for download sources
--     Call download.dltable() to create a new download table.
--     Use dltable:addsource('source') to add a source to the table.
--     dltable['source'] uses both date() and date().daynum as hash keys.
-- ============================================================================

require 'date'

-- A download hash table
-- dltable["sourcename"] = datetable
local mt = {}
mt.__index = mt
function download.dltable()
    return setmetatable({}, mt)
end

-- Use this function to correctly add a new source table (as a datetable)
function mt:addsource(src)
    rawset(self, src, download.datetable())
end

--[[
-- Force the user to use dltable:addsource, unless they are deleting the index
function mt:__newindex(index, val)
    if val then
        error('use addsource')
    else
        rawset(index, obj, nil)
    end
end
]]

-- Convert date() <--> date().daynum
local function from_date(d)
    return d.daynum
end

local function to_date(daynum)
    -- date.epoch() is the cleanest way to create a new date with
    -- dayfrc already set to 0
    local d = date.epoch()
    d.daynum = daynum
    return d
end


--[[
A date hash table
Usage:
    datetable[date()] = dlctrl
    datetable[date().daynum = dlctrl

    datetable[date()] => dlctrl
    datetable[date().daynum] => dlctrl

Note that dates are stored without time.
e.g.
    datetable[date{year=2000, month=1, day=1, hour=12}] = 'testing'
    datetable[date{year=2000, month=1, day=1, hour=6}] => 'testing'
    datetable[date{year=2000, month=1, day=2] => nil
 ]]
function download.datetable()
    local d = {}   -- [date()] = download ctrl

    -- A table mapping daynuum values to the date objects in the datetable.
    -- This is completely private, as it is wrapped in the enclosure of
    -- function datetable()
    local datemap = {} -- [daynum] = date()

    -- The metatable
    local datemt = {}

    -- We can index the datetable with either the raw date.daynum or the date
    -- object itself
    function datemt:__index(num)
        if type(num) == "table" then
            assert(num.daynum, "datetable indicies must be date objects")
            num = num.daynum
        end

        -- Look for the daynum to date conversion in the datemap
        local d = datemap[num]
        -- If num is not in the datemap, it is not in the datetable at all
        if not d then return nil end
        -- This should be guaranteed to return non-nil
        return rawget(self, d)
    end

    -- Add a new index with a date object or a date.daynum
    function datemt:__newindex(d, val)
        if type(d) == "number" then
            d = to_date(d) -- Assume this is a daynum
        else
            assert(type(d) == "table" and d.daynum,
                   "datetable indicies must be date objects")
            -- Only story the date, not the time
            d.dayfrc = 0
        end

        -- Get the *real* date object if it is already in the table
        local daynum = from_date(d)
        d = datemap[daynum] or d:copy()

        rawset(self, d, val)
        if val then -- store the conversion from daynum to the real date object
            datemap[daynum] = d
        else -- remove the date from the datemap if val is nil
            datemap[daynum] = nil
        end
    end

    --d.datemap = datemap  -- for debug purposes
    return setmetatable(d, datemt)
end
