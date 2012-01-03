local function rexparker(d)
    -- We have to "search" for entries that are earlier than a given date, so
    -- we need to search one past today.
    d:adddays(1)
    blog.open("http://rexwordpuzzle.blogspot.com/search?updated-max=%Y-%m-%dT00:00:00-05:00&max-results=1", d)
end

blog.add_entry('Rex Parker Does the NYT Crossword Puzzle', rexparker)
