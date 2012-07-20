require 'luacurl'

local c = curl.easy_init()
c:setopt(curl.OPT_FAILONERROR, 1) -- e.g. 404 errors
c:setopt(curl.OPT_FOLLOWLOCATION, 1)

-- return a table { { word = word, confidence = confidence }, ... }
function download_suggestions(url)
    -- Download
    -- Callback for WRITEFUNCTION
    local text = {}
    function saveText(str, length)
        table.insert(text, str)
        return length
    end
    c:setopt(curl.OPT_URL, url)
    c:setopt(curl.OPT_WRITEFUNCTION, saveText)

    local success, err = c:perform()
    if not success then task.debug("error: " .. err) return end
    text = table.concat(text)

    -- Parse
    local ret = {}
    -- Extract the body of the matches table
    text = text:match("<b>Source</b></font></td></tr>(.-)</table>")
    if not text then task.debug("Could not parse") return {} end
    -- Move through the rows of the table
    for row in text:gmatch("<tr(.-)</tr>") do
        -- Count the number of stars this gets (rank)
        local value, count = row:gsub('<img src="/images/smallstarblack.gif">', '')
        -- Find the word
        local word = row:match("<a href.->(.-)</a>")
        -- Add to the master list
        if count > 0 and word then
            table.insert(ret, {word = word, confidence = count})
        end
    end
    return ret
end

-- Download the given words
for idx, url in pairs(arg) do
    -- Try each word 5 times before we give up
    local suggestions = download_suggestions(url)
    for i=1,5 do
        if suggestions and #suggestions > 0 then break end
        suggestions = download_suggestions(url)
    end
    task.post(1, {idx, suggestions}, 10)
    if task.checkAbort() then break end
end