-- General options
USE_LUA = false

-- Windows options
WXWIN = "$(WXWIN)"

-- Mac options
WXMAC = "/Developer/wxMac-2.8.10"
WXMAC_BUILD_DEBUG = WXMAC .. "/build-debug"
WXMAC_BUILD_RELEASE = WXMAC .. "/build-release"


-- Don't mess with anything below this

function split(text, delims)
    delims = delims or " \t\r\f\n"
    local t = {}
    local patt = "([^" .. delims .. "]+)
    for s in text:gmatch(patt) do
        table.insert(t, s)
    end
    return t
end

-- Execute the command specified and return the result, correctly formatted
function cmd(cmd_)
    return table.concat(split(os.outputof(cmd_)), " ")
end