require 'luacurl'

-- ============================================================================
-- Sources
-- ============================================================================

function download.get_default_puzzles()
local sources = {
    {
        name = "NY Times Premium",
        url = "http://select.nytimes.com/premium/xword/%Y/%m/%d/%b%d%y.puz",
        directoryname = "NY Times",
        filename = "nyt%Y%m%d.puz",
        days = { true, true, true, true, true, true, true },
        fields = { "User Name", "Password", },
        func = [[
    local cookies_filename = path.join(xword.configdir, 'download', 'nytcookie.txt')
    -- Look for the cookies
    local function has_cookie()
        local f = io.open(cookies_filename, 'rb')
        if not f then return false end
        local has_cookie = f:read("*all"):match("NYT%-S")
        f:close()
        return has_cookie and true or false
    end
    if not has_cookie() then
        if not puzzle.user_name or not puzzle.password then
            return "User name or password not specified"
        end
        set_status("Logging in to NY Times")
        -- Download authentication page
        local auth = download("https://myaccount.nytimes.com/auth/login")
        -- Find the form
        local form = auth:match("<form(.-)</form>")
        -- Look for the <input>s and assemble the POST string
        local postdata = {}
        for data in form:gmatch("<input([^>]*)>") do
            local name = data:match('name%s*=%s*"(.-)"')
            if name ~= "userid" and name ~= "password" then
                local value = data:match('value%s*=%s*"(.-)"')
                table.insert(postdata, curl.escape(name) .. "=" .. curl.escape(value))
            end
        end
        table.insert(postdata, "userid=" .. curl.escape(puzzle.user_name))
        table.insert(postdata, "password=" .. curl.escape(puzzle.password))
        -- Send the POST request and save the cookie
        local str, c = download("https://myaccount.nytimes.com/auth/login",
                { [curl.OPT_POST] = 1,
                  [curl.OPT_POSTFIELDS] = table.concat(postdata, "&"),
                  [curl.OPT_COOKIEJAR] = cookies_filename })
        -- Flush the cookies
        -- Old versions of luacurl don't let you use OPT_COOKIELIST, so
        -- it's OK if this fails.
        if pcall(function () c:setopt(curl.OPT_COOKIELIST, "FLUSH") end) then
            if not has_cookie() then
                return "Unable to login"
            end
        end
    end
    -- Download the puzzle, using the supplied cookies
    set_status()
    download(puzzle.url, puzzle.filename,
             { [curl.OPT_COOKIEFILE] = cookies_filename })
]]
    },

    {
        name = "NY Times (XWord Info)",
        url = "http://www.xwordinfo.com/XPF/?date=%m/%d/%Y",
        directoryname = "NY Times",
        filename = "nyt%Y%m%d.xml",
        days = { true, true, true, true, true, true, true },
        curlopts =
        {
            [curl.OPT_REFERER] = 'http://www.xwordinfo.com/',
        },
        func = [[
    -- Try to download the XPF first
    download(puzzle.url, puzzle.filename, puzzle.curlopts)
    if true then return end -- Cut this off for now.
    local success, p = pcall(puz.Puzzle, puzzle.filename)
    if success then
        p:__gc()
        return
    end
    -- Otherwise download it as JSON
    local url = puzzle.date:fmt("http://www.xwordinfo.com/JSON/Data.aspx?date=%m/%d/%Y")
    download(url, puzzle.filename, puzzle.curlopts)
    local success, p = pcall(puz.Puzzle, puzzle.filename, import.xwordinfoJSON)
    if success then
        p:Save(puzzle.filename) -- Save this as an XPF
        p:__gc()
    end
]]
    },

    {
        name = "CrosSynergy",
        url = "http://www.washingtonpost.com/r/WashingtonPost/Content/Puzzles/Daily/cs%y%m%d.jpz",
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
        url = "http://mazerlm.home.comcast.net/pi%y%m%d.puz",
        filename = "pi%Y%m%d.puz",
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
        url = "http://crosswordcontest.blogspot.com/%Y_%m_%d_archive.html",
        -- Custom download function
        func = [[
    -- Sometimes the puzzles are released early
    puzzle.date:adddays(-4)
    puzzle.url = puzzle.date:fmt("http://crosswordcontest.blogspot.com/%Y_%m_%d_archive.html")
    -- Download the page with puzzles after today's date
    local archive = download(puzzle.url)

    -- Find the last Across Lite applet
    local id, name
    for a,b in archive:gmatch('"http://icrossword.com/embed/%?id=([^"]*)(mgwcc[^"]*)"') do
        id, name = a,b
    end
    if id and name then
        download(
            string.format("http://icrossword.com/publish/server/puzzle/index.php/%s?id=%s%s",
                          name, id, name),
            puzzle.filename)
    else
        return "Could not find a puzzle."
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
    local archive = download(puzzle.url)

    -- Find the last Across Lite applet
    local id, name
    for a,b in archive:gmatch('"http://icrossword.com/embed/%?id=([^"]*)(mgdc[^"]*)"') do
        id, name = a,b
    end
    if id and name then
        download(
            string.format("http://icrossword.com/publish/server/puzzle/index.php/%s?id=%s%s",
                          name, id, name),
            puzzle.filename)
    else
        return "Could not find a puzzle."
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
    local page = download(puzzle.url)

    -- Search for a download link
    local url = page:match('<a href="(http://www.brendanemmettquigley.com/[^"]-.jpz)">')

    -- Download the puzzle
    if url then
        download(url, puzzle.filename)
    else
        return "Could not find a download link"
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
        url = "http://crosswords.washingtonpost.com/wp-srv/style/crosswords/util/csserve2.cgi/cs%y%m%d.puz?t=pimage&z=puzzler&f=cs%y%m%d.puz",
        filename = "wp%Y%m%d.puz",
        days = { false, false, false, false, false, false, true },
    },
}

-- Make this into a class
local puzzles = {}
puzzles._order = {}

local mt = {}
mt.__index = mt

-- Iterate enabled puzzles
function mt:iter()
    local i = 0
    local n = #self._order
    return function ()
        while i <= n do
            i = i + 1
            if i <= n then
                local key = self._order[i]
                if download.isenabled(self[key]) then
                    return key, self[key]
                end
            end
        end
    end
end

-- Iterate all puzzles
function mt:iterall()
    local i = 0
    local n = #self._order
    return function ()
        i = i + 1
        if i <= n then
            local key = self._order[i]
            return key, self[key]
        end
    end
end

function mt:get(i)
    local p = self[i]
    if p then return p end
    local key = self._order[i]
    if key then return self[key] end
end

function mt:insert(puzzle, idx)
    -- Generate an id
    puzzle.id = puzzle.id or puzzle.name
    local i = 2
    while self[puzzle.id] ~= nil do
        puzzle.id = puzzle.name .. tostring(i)
        i = i + 1
    end
    -- Insert the puzzle
    self[puzzle.id] = puzzle
    if idx then
        table.insert(self._order, idx, puzzle.id)
    else
        table.insert(self._order, puzzle.id)
    end
end

function mt:remove(id)
    if type(id) == 'table' then
        id = id.id
    end
    for i, puzid in ipairs(self._order) do
        if puzid == id then
            table.remove(self._order, i)
        end
    end
    self[id] = nil
end

setmetatable(puzzles, mt)

for _, puzzle in ipairs(sources) do
    puzzles:insert(puzzle)
end
for k,_ in pairs(sources) do sources[k] = nil end

return puzzles

end -- function get_default_puzzles

download.puzzles = download.get_default_puzzles()

-- ============================================================================
-- Source-related functions
-- ============================================================================
local deepcopy = require 'pl.tablex'.deepcopy
local join = require 'pl.path'.join

function download.sanitize_name(text)
    -- Remove characters that are not allowed in filenames
    local text = text:gsub('[?<>:*|"%%]\t\r\n\v\f', "")
    return text
end

function download.get_url(puzzle, d)
    if type(puzzle.url) == 'string' then
        return d:fmt(puzzle.url)
    else
        return puzzle.url
    end
end

function download.get_filename(puzzle, d)
    if download.separate_directories then
        return join(download.puzzle_directory, download.sanitize_name(puzzle.directoryname or puzzle.name), d:fmt(puzzle.filename))
    else
        return join(download.puzzle_directory, d:fmt(puzzle.filename))
    end
end

function download.get_download_data(puzzle, d)
    local data = deepcopy(puzzle)
    if data.url then data.url = download.get_url(puzzle, d) end
    if data.filename then data.filename = download.get_filename(puzzle, d) end
    data.date = d:copy()
    return data
end

function download.isdisabled(puzzle)
    return (download.disabled or {})[puzzle.id] == true
end

function download.isenabled(puzzle)
    return not download.isdisabled(puzzle)
end


-- Update the download sources
require 'download.config'
