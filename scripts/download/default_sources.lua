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
        url = "https://cdn4.amuselabs.com/lat/crossword?id=tca%y%m%d&set=latimes",
        filename = "lat%Y%m%d.jpz",
        days = { true, true, true, true, true, true, true },
        func = [[
    local picker = assert(curl.get("https://cdn4.amuselabs.com/lat/date-picker?style=1&embed=1&heightReduction=30&set=latimes"))
    local param_str = picker:match("pickerParams.rawsps%s*=%s*'([^']+)'")
    if not param_str then
        param_str = picker:match('"rawsps"%s*:%s*"([^"]+)"')
        if not param_str then
            return "Unable to find puzzle"
        end
    end
    local params = json.to_value(base64.decode(param_str))

    local page = assert(curl.get(puzzle.url .. "&pickerToken=" .. params.pickerToken))
    local p = puz.Puzzle(page, import.amuselabsHtml)
    p:Save(puzzle.filename)
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
          local amuse_url = page:match('http[^ ]*amuselabs.com/[^ ]-embed=1')
          if not amuse_url then
              return "No puzzle"
          end
          local page2 = assert(curl.get(amuse_url))

          local p = puz.Puzzle(page2, import.amuselabsHtml)
          p:Save(puzzle.filename)
      ]]
    },

    {
      name = "The New Yorker Cryptic",
      url = "https://www.newyorker.com/puzzles-and-games-dept/cryptic-crossword/%Y/%m/%d",
      filename = "nyer-cryptic%Y%m%d.jpz",
      days = { false, false, false, false, false, false, true },
      func = [[
          -- Download the page with the puzzle
          local page = assert(curl.get(puzzle.url))

          -- Search for the app code
          local amuse_url = page:match('http[^ ]*amuselabs.com/[^ ]-embed=1')
          if not amuse_url then
              return "No puzzle"
          end
          local page2 = assert(curl.get(amuse_url))

          local p = puz.Puzzle(page2, import.amuselabsHtml)
          p:Save(puzzle.filename)
      ]]
    },

    {
      name = "The Atlantic",
      url = "https://cdn3.amuselabs.com/atlantic/crossword?id=atlantic_%Y%m%d&set=atlantic&embed=1",
      filename = "atlantic%Y%m%d.jpz",
      days = { true, true, true, true, true, false, true },
      func = [[
          local page = assert(curl.get(puzzle.url))
          local p = puz.Puzzle(page, import.amuselabsHtml)
          p:Save(puzzle.filename)
      ]]
    },

    {
      name = "New York Magazine",
      url = "https://nymag.com/crossword",
      filename = "nym%Y%m%d.jpz",
      days = { false, false, false, false, false, false, true },
      func = [[
          -- Download the puzzle list
          local puzzle_page_url
          for start=0,20,20 do
              print(puzzle.url .. "?start=" .. start)
              local page = assert(curl.get(puzzle.url .. "?start=" .. start))

              local datestr = puzzle.date:fmt('%b%%S* %d,? %Y'):gsub(" 0", " ")

              puzzle_page_url = page:match(datestr .. '.-href="([^"]+)".-[Cc]rossword')
              if puzzle_page_url then break end
          end
          if not puzzle_page_url then
            return "No Puzzle"
          end
          puzzle_page_url = puzzle_page_url:gsub("^//", "https://")
          print("nymag", puzzle_page_url)

          local page2 = assert(curl.get(puzzle_page_url))
          local amuse_url = page2:match('data%-crossword%-url%s*=%s*"([^"]+)"')
          if not amuse_url then
              return "No puzzle"
          end
          print("nymag amuse", amuse_url)

          local decoded_url = amuse_url:gsub("&#x3[dD];", "="):gsub("&amp;", "&"):gsub(" ", "+")
          local page3 = assert(curl.get(decoded_url))

          local p = puz.Puzzle(page3, import.amuselabsHtml)
          p:Save(puzzle.filename)
      ]]
    },
    {
      name = "Vulture",
      url = "https://www.vulture.com/%Y/%m/daily-crossword-puzzle-%B-%d.html",
      filename = "nym%Y%m%d.jpz",
      days = { true, true, true, true, true, false, false },
      func = [[
          local page = curl.get(puzzle.url:gsub("0(%d.html)", "%1"):lower())
          if not page then
            alt_url = puzzle.date:fmt("https://www.vulture.com/%Y/%m/daily-crossword-puzzle-%A-%B-%d.html")
            page = assert(curl.get(alt_url:gsub("0(%d.html)", "%1"):lower()))
          end

          local amuse_url = page:match('data%-crossword%-url%s*=%s*"([^"]+)"')
          if not amuse_url then
              return "No puzzle"
          end

          -- a little html decoding and url encoding
          local decoded_url = amuse_url:gsub("&#x3[dD];", "="):gsub("&amp;", "&"):gsub(" ", "+")
          local page2 = assert(curl.get(decoded_url))

          local p = puz.Puzzle(page2, import.amuselabsHtml)
          p:Save(puzzle.filename)
      ]]
    },
}
