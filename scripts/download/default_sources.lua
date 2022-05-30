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
    local p = puz.Puzzle(puzzle.filename, import.Newsday)
    p:Save(puzzle.filename) -- Save this as a puz
]]
    },

    {
        name = "LA Times",
        url = "http://cdn.games.arkadiumhosted.com/latimes/assets/DailyCrossword/la%y%m%d.xml",
        filename = "lat%Y%m%d.jpz",
        days = { true, true, true, true, true, true, true },
        func = [[
          -- Hack: strip out "format", which is used for enumerations
          local xml = assert(curl.get(puzzle.url))
          xml = xml:gsub('format%s*=%s*"[^"]+"', '')
          xml = xml:gsub("format%s*=%s*'[^']+'", "")
          local f = assert(io.open(puzzle.filename, 'wb'))
          f:write(xml)
          f:close()
        ]]
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
        name = "Universal Daily",
        url = "http://picayune.uclick.com/comics/fcx/data/fcx%y%m%d-data.xml",
        filename = "univ%Y%m%d.xml",
        days = { true, true, true, true, true, true, true },
    },

    {
          name = "Universal Sunday",
          url = "http://herbach.dnsalias.com/uc/ucs%y%m%d.puz",
          filename = "ucs%Y%m%d.puz",
          days = { false, false, false, false, false, false, true },
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
            local name = page:match('www.brendanemmettquigley.com/javaapp/([^"]-).html"')
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
            local jpz_url = page:match('www.brendanemmettquigley.com/files/[^"]+%.jpz')
            if jpz_url then
                assert(curl.get('http://' .. jpz_url, puzzle.filename))
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

    {
      name = "The New Yorker",
      url = "https://www.newyorker.com/puzzles-and-games-dept/crossword/%Y/%m/%d",
      filename = "nyer%Y%m%d.jpz",
      days = { true, true, true, true, true, false, false },
      func = [[
          -- Download the page with the puzzle
          local page = assert(curl.get(puzzle.url))

          -- Search for the app code
          local amuse_url = page:match('http[^ ]*amuselabs.com/[^ ]+embed=1')
          if not amuse_url then
              return "No puzzle"
          end
          local page2 = assert(curl.get(amuse_url))

          -- Get the crossword data
          local data = page2:match("rawc%s*=%s*'([^']+)'")
          if data then
            -- Using the data _as_ the input filename is kind of a hack, but it works
            local p = puz.Puzzle(data, import.amuselabsBase64)
            p:Save(puzzle.filename)
          else
            return "Unable to find puzzle data"
          end
      ]]
    },

    {
      name = "The Atlantic",
      url = "https://cdn3.amuselabs.com/atlantic/crossword?id=atlantic_%Y%m%d&set=atlantic&embed=1",
      filename = "atlantic%Y%m%d.jpz",
      days = { true, true, true, true, true, false, true },
      func = [[
          -- Download the page with the puzzle
          local page = assert(curl.get(puzzle.url))

          -- Get the crossword data
          local data = page:match("rawc%s*=%s*'([^']+)'")
          if data then
            local p = puz.Puzzle(data, import.amuselabsBase64)
            p:Save(puzzle.filename)
          else
            return "Unable to find puzzle data"
          end
      ]]
    },

    {
      name = "New York Magazine",
      url = "https://cdn3.amuselabs.com/nymag/date-picker?set=nymag&theme=nymag&embed=1&limit=100",
      filename = "nym%Y%m%d.jpz",
      days = { false, false, false, false, false, false, true },
      func = [[
          -- Download the puzzle list
          local page = assert(curl.get(puzzle.url))
          local puz_datestr = puzzle.date:fmt('%d %B %Y'):gsub('^0', '')

          -- Find the url for this date
          local amuse_url
          for id, datestr in page:gmatch('data%-id="(.-)".-<strong>(.-)</strong>') do
            if datestr == puz_datestr then
              amuse_url = "https://cdn3.amuselabs.com/nymag/crossword?id=" .. id .. "&set=nymag&embed=1"
              break
            end
          end
          if not amuse_url then
            return "No Puzzle"
          end

          -- Download the puzzle page
          local page2 = assert(curl.get(amuse_url))

          -- Get the crossword data
          local data = page2:match("rawc%s*=%s*'([^']+)'")
          if data then
            local p = puz.Puzzle(data, import.amuselabsBase64)
            p:Save(puzzle.filename)
          else
            return "Unable to find puzzle data"
          end
      ]]
    },
}
