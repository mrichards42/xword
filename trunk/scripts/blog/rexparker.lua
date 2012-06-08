local function rexparker(d)
    blog.open("http://rexwordpuzzle.blogspot.com/search?updated-max=%Y-%m-%dT00:00:00-05:00&max-results=1", d)
end

blog.add_entry('Rex Parker Does the NYT Crossword Puzzle', rexparker)
