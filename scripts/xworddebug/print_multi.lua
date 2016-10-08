local date = require 'date'
local sources = require 'download.sources'
local nyt = sources['NY Times']
-- Only Monday or Tuesday
local default_weekdays = {true, true, false, false, false, false, false}

--- Print multiple puzzles
-- @param d Start date
-- @param end_ End date
-- @param[opt=NY Times] src
-- @param[opt] weekdays A map of valid isoweekdays
function print_multi(d, end_, src, weekdays)
    assert(d and end_)
    src = src or nyt
    weekdays = weekdays or default_weekdays
    assert(src)
    local info = nil
    while d < end_ do
        if weekdays[d:getisoweekday()] then
            local puzzle = src:get_puzzle(d)
            if puzzle:exists() then
                local success, p = pcall(puz.Puzzle, puzzle.filename)
                if success and p then
                    if not info then -- prompt once
                        info = xword.PrintInfo(xword.DRAW_NUMBER)
                        print(d, p)
                        if not xword.frame:Print(info, p) then
                            break
                        end
                    else
                        print(d, p)
                        xword.frame:Print(info, p, false) -- don't prompt
                    end
                end
            end
        end
        d = d:adddays(1)
    end
end

return print_multi