local serialize = require 'serialize'
local lfs = require 'lfs'
local join = require 'pl.path'.join
local startswith = require 'pl.stringx'.startswith

local function write_html(filename)
    local p = xword.frame.Puzzle

    local html = {
    [[<html><head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
    <style>
    /* puzzle info */
    .title {
        font-size: 1.5em;
    }

    .author {
        font-size: 1.2em;
        font-style: italic;
    }

    /* grid */
    table {
        border-collapse:collapse;
    }

    table.grid td {
        border: 1px solid black;
        width: 30px;
        height: 30px;
        padding: 0;
        margin: 0;
        vertical-align: top;
    }

    .black {
        background: black;
    }

    div.number {
        font-size: 10px;
        margin: 1px;
    }

    /* clues */
    table.clues {
        float: left;
        width: 300px;
    }

    table.clues td {
        padding: 2px;
        vertical-align: top;
    }
    td.number {
        font-weight: bold;
        text-align: right
    }
    </style>
    </head><body>
    ]]
    }

    -- Heading info
    table.insert(html, "<div class='title'>" .. p.Title .. "</div>")
    table.insert(html, "<div class='author'>" .. p.Author .. "</div>")

    -- Generate the grid
    table.insert(html, "<table class='grid'>")
    local s = p.Grid:First()
    while s do
        if s:IsFirst(puz.ACROSS) then
            table.insert(html, "<tr>")
        end
        table.insert(html, "<td class='" .. (s:IsBlack() and 'black' or 'white') .. "'>")
        if s:HasNumber() then
            table.insert(html, "<div class='number'>" .. s.Number .. "</div>")
        end
        table.insert(html, "</td>")
        if s:IsLast(puz.ACROSS) then
            table.insert(html, "</tr>")
        end
        s = s:Next()
    end
    table.insert(html, "</table>")

    -- Generate the clues
    local function make_clues(direction)
        local clues = { "<table class='clues'>" }
        table.insert(clues, "<tr><th colspan=2>" .. direction .. "</th></tr>")
        local list = p:GetClueList(direction)
        for _, clue in ipairs(list) do
            table.insert(clues, "<tr>")
            table.insert(clues, "<td class='number'>" .. clue.number .. "</td>")
            table.insert(clues, "<td class='clue'>" .. clue.text .. "</td>")
            table.insert(clues, "</tr>")
        end
        table.insert(clues, "</table>")
        return table.concat(clues)
    end

    table.insert(html, make_clues("Across"))
    table.insert(html, make_clues("Down"))

    table.insert(html, "</body></html>")

    -- Write to a file
    local f = assert(io.open(filename, 'wb'))
    f:write(table.concat(html))
    f:close()

    wx.wxLaunchDefaultBrowser('file:///' .. filename)
end

local function init()
    xword.frame:AddMenuItem(
        {'Debug'}, "Generate HTML Puzzle",
        function(evt)
            if not xword.HasPuzzle() then return end
            local filename = wx.wxFileSelector(
                "Save HTML",
                xword.userdatadir,
                "puzzle.html", "html",
                "*.html", wx.wxFD_SAVE
            )
            if filename and filename ~= '' then
                write_html(filename)
            end
        end
    )
end

if xword.frame then
    init()
end
