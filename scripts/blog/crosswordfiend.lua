local function fiend(d)
    -- Day should not have a leading zero
    local dayfmt = d:fmt("%d")
    if dayfmt:sub(1,1) == "0" then
        dayfmt = dayfmt:sub(2,2)
    end
    blog.open("http://www.crosswordfiend.com/blog/%A-%m" .. dayfmt .. "%y/", d)
end

blog.add_entry('Diary Of A Crossword Fiend', fiend)
