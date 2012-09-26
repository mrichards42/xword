local function wordplay(d)
    d:adddays(-1) -- Wordplay is always a day ahead.
    blog.open("http://wordplay.blogs.nytimes.com/%Y/%m/%d", d)
end

blog.add_entry('Wordplay', wordplay)
