return {
    {
        name = "NY Times Premium",
        url = "http://select.nytimes.com/premium/xword/%Y/%m/%d/%b%d%y.puz",
        directoryname = "NY Times",
        filename = "nyt%Y%m%d.puz",
        days = { true, true, true, true, true, true, true },
        auth = { url="https://myaccount.nytimes.com/auth/login", user_id="userid", password_id="password"},
    },

    {
        name = "NY Times PDF",
        url = "http://select.nytimes.com/premium/xword/%Y/%m/%d/%b%d%y.pdf",
        directoryname = "NY Times",
        filename = "nyt%Y%m%d.pdf",
        days = { true, true, true, true, true, true, true },
        not_puzzle = true,
    },

    {
        name = "Newsday",
        url = "http://www.brainsonly.com/servlets-newsday-crossword/newsdaycrossword?date=%y%m%d",
        filename = "nd%Y%m%d.puz",
        days = { true, true, true, true, true, true, true },
        func = [[
    assert(curl.get(puzzle.url, puzzle.filename, puzzle.curlopts))
    local success, p = pcall(puz.Puzzle, puzzle.filename, import.Newsday)
    if success then
        p:Save(puzzle.filename) -- Save this as a puz
    end
]]
    },

    {
        name = "LA Times",
        url = "http://cdn.games.arkadiumhosted.com/latimes/assets/DailyCrossword/la%y%m%d.xml",
        filename = "lat%Y%m%d.jpz",
        days = { true, true, true, true, true, true, true },
    },

    {
        name = "USA Today",
        url = "http://picayune.uclick.com/comics/usaon/data/usaon%y%m%d-data.xml",
        filename = "usa%Y%m%d.xml",
        days = { true, true, true, true, true, true, true },
    },

    {
        name = "Jonesin'",
        url = "http://herbach.dnsalias.com/Jonesin/jz%y%m%d.puz",
        filename = "jones%Y%m%d.puz",
        days = { false, false, false, true, false, false, false },
    },

    {
        name = "Wall Street Journal",
        url = "http://herbach.dnsalias.com/wsj/wsj%y%m%d.puz",
        filename = "wsj%Y%m%d.puz",
        days = { true, true, true, true, true, true, false },
    },

    {
        name = "Universal",
        url = "http://picayune.uclick.com/comics/fcx/data/fcx%y%m%d-data.xml",
        filename = "univ%Y%m%d.xml",
        days = { true, true, true, true, true, true, true },
    },

    {
        name = "Brendan Emmett Quigley",
        url = "http://www.brendanemmettquigley.com/%Y/%m/%d",
        filename = "beq%Y%m%d.jpz",
        days = { true, false, false, true, false, false, false },
        -- Custom download function
        func = [[
            -- Download the page with the puzzle
            local page = assert(curl.get(puzzle.url))

            -- Search for a download link (crossword solver app)
            local name = page:match('src="http://www.brendanemmettquigley.com/javaapp/([^"]-).html"')
            if name then
                -- Download the puzzle as an jpz javascript
                local js = assert(curl.get("http://www.brendanemmettquigley.com/xpuz/" .. name .. ".js"))

                -- Extract the string from this js
                local jpz = js:match("['\"](.+)['\"]")
                local f = assert(io.open(puzzle.filename, 'wb'))
                f:write(jpz:gsub('\\"', '"')) -- Unescape strings
                f:close()
                return
            end

            -- Search for a jpz link
            local jpz_url = page:match('href="(http://www.brendanemmettquigley.com/files/[^"]+%.jpz)"')
            if jpz_url then
                assert(curl.get(jpz_url, puzzle.filename))
                return
            end
            return "No puzzle"
        ]]
    },

    {
        name = "Washington Post Sunday",
        url = "http://herbach.dnsalias.com/WaPo/wp%y%m%d.puz",
        filename = "wp%Y%m%d.puz",
        days = { false, false, false, false, false, false, true },
    },
}
