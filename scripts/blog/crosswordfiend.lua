local function chop(str)
    if str:sub(1,1) == "0" then
        return str:sub(2)
    else
        return str
    end
end

local function fiend(d)
    -- Day should not have a leading zero
    local day = chop(d:fmt("%d"))
    local month = chop(d:fmt("%m"))

    blog.open("http://www.crosswordfiend.com/blog/%A-" .. month .. day .. "%y/", d)
end

blog.add_entry('Diary Of A Crossword Fiend', fiend)
