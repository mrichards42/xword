local function xwordinfo(d)
    blog.open("http://www.xwordinfo.com/Crossword?date=%m/%d/%Y", d)
end

blog.add_entry('XWord Info', xwordinfo)
