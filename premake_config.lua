-- General options
USE_LUA = true

-- Windows options
WXWIN = "$(WXWIN)"

-- Mac options
WXMAC = "/Developer/wxMac-2.8.10"
WXMAC_BUILD_DEBUG = WXMAC .. "/build-debug"
WXMAC_BUILD_RELEASE = WXMAC .. "/build-release"


-- Don't mess with anything below this
function trim(text)
    return string.match(text, "[ \t\r\f\n]*(.*)[ \t\r\f\n]*")
end