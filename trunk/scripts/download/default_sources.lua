-- Default puzzle sources

local sources = {
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
        name = "CrosSynergy",
        url = "http://cdn.games.arkadiumhosted.com/washingtonpost/crossynergy/cs%y%m%d.jpz",
        filename = "cs%Y%m%d.jpz",
        days = { true, true, true, true, true, true, true },
    },

    {
        name = "Newsday",
        url = "http://picayune.uclick.com/comics/crnet/data/crnet%y%m%d-data.xml",
        filename = "nd%Y%m%d.xml",
        days = { true, true, true, true, true, true, true },
    },

    {
        name = "LA Times",
        url = "http://www.cruciverb.com/puzzles/lat/lat%y%m%d.puz",
        filename = "lat%Y%m%d.puz",
        curlopts =
        {
            [curl.OPT_REFERER] = 'http://www.cruciverb.com/',
        },
        days = { true, true, true, true, true, true, true },
    },

    {
        name = "USA Today",
        url = "http://picayune.uclick.com/comics/usaon/data/usaon%y%m%d-data.xml",
        filename = "usa%Y%m%d.xml",
        days = { true, true, true, true, true, false, false },
    },
    {
        name = "Ink Well",
        url = "http://herbach.dnsalias.com/Tausig/vv%y%m%d.puz",
        filename = "tausig%Y%m%d.puz",
        days = { false, false, false, false, true, false, false },
    },

    {
        name = "The Onion AV Club",
        url = "http://herbach.dnsalias.com/Tausig/av%y%m%d.puz",
        filename = "av%Y%m%d.puz",
        days = { false, false, true, false, false, false, false },
    },

    {
        name = "Jonesin'",
        url = "http://herbach.dnsalias.com/Jonesin/jz%y%m%d.puz",
        filename = "jones%Y%m%d.puz",
        days = { false, false, false, true, false, false, false },
    },

    {
        name = "Wall Street Journal",
        url = "http://mazerlm.home.comcast.net/wsj%y%m%d.puz",
        filename = "wsj%Y%m%d.puz",
        days = { false, false, false, false, true, false, false },
    },

    {
        name = "Boston Globe",
        url = "http://home.comcast.net/~nshack/Puzzles/bg%y%m%d.puz",
        filename = "bg%Y%m%d.puz",
        days = { false, false, false, false, false, false, true },
    },

    {
        name = "Philadelphia Inquirer",
        url = "http://cdn.games.arkadiumhosted.com/latimes/assets/SundayCrossword/mreagle_%y%m%d.xml",
        filename = "pi%Y%m%d.jpz",
        days = { false, false, false, false, false, false, true },
    },

    {
        name = "The Chronicle of Higher Education",
        url = "http://chronicle.com/items/biz/puzzles/%Y%m%d.puz",
        filename = "che%Y%m%d.puz",
        days = { false, false, false, false, true, false, false },
    },

    {
        name = "Universal",
        url = "http://picayune.uclick.com/comics/fcx/data/fcx%y%m%d-data.xml",
        filename = "univ%Y%m%d.xml",
        days = { true, true, true, true, true, true, true },
    },

    {
        name = "Matt Gaffney's Weekly Crossword Contest",
        filename = "mgwcc%Y%m%d.puz",
        days = { false, false, false, false, true, false, false },
        url = "http://xwordcontest.com/%Y/%m/%d",
        curlopts =
        {
            [curl.OPT_REFERER] = 'http://icrossword.com/',
        },
        -- Custom download function
        func = [[
    -- Download the page with this week's puzzle
    local page = assert(curl.get(puzzle.url))

    -- Find the Across Lite applet
    local id = page:match('"http://icrossword.com/embed/%?id=([^"]*%.puz)"')
    if id then
        local url = "http://icrossword.com/publish/server/puzzle/serve.php/?id=" .. id
        return curl.get(url, puzzle.filename)
    else
        return "No puzzle"
    end
]]
    },

    {
        name = "Matt Gaffney's Daily Crossword",
        filename = "mgdc%Y%m%d.puz",
        days = { true, true, true, true, true, false, false },
        url = "http://mattgaffneydaily.blogspot.com/%Y_%m_%d_archive.html",
        -- Custom download function
        func = [[
    -- Download the page with puzzles after today's date
    local archive = assert(curl.get(puzzle.url))

    -- Find the last Across Lite applet
    local id, name
    for a,b in archive:gmatch('"http://icrossword.com/embed/%?id=([^"]*)(mgdc[^"]*)"') do
        id, name = a,b
    end
    if id and name then
        local url = string.format(
            "http://icrossword.com/publish/server/puzzle/index.php/%s?id=%s%s",
            name, id, name
        )
        return curl.get(url, puzzle.filename)
    else
        return "No puzzle"
    end
]]
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

    -- Search for a download link
    local name = page:match('src="http://www.brendanemmettquigley.com/javaapp/([^"]-).html"')

    -- Download the puzzle
    if name then
        local url = "http://www.brendanemmettquigley.com/xpuz/" .. name .. ".jpz"
        return curl.get(url, puzzle.filename)
    else
        return "No puzzle"
    end 
]]
    },

    {
        name = "I Swear",
        url = "http://wij.theworld.com/puzzles/dailyrecord/DR%y%m%d.puz",
        filename = "dr%Y%m%d.puz",
        days = { false, false, false, false, true, false, false },
    },

    {
        name = "Washington Post Puzzler",
        url = "http://cdn.games.arkadiumhosted.com/washingtonpost/puzzler/puzzle_%y%m%d.xml",
        filename = "wp%Y%m%d.jpz",
        days = { false, false, false, false, false, false, true },
        -- Custom download function
        func = [[
    local jpz = assert(curl.get(puzzle.url))
    local f = assert(io.open(puzzle.filename, 'wb'))
    f:write(jpz:gsub("crossword-compiler", "crossword-compiler-applet"))
    f:close()
]]
},
}

-- Copy NY Times custom download function for NY Times PDF
sources[2].func = sources[1].func

return sources