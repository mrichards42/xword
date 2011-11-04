-- General options
USE_LUA = true

-- Windows options
WXWIN = "$(WXWIN)"

-- Mac options
WXMAC = "/Developer/wxMac-2.8.10"
WXMAC_BUILD_DEBUG = WXMAC .. "/build-debug"
WXMAC_BUILD_RELEASE = WXMAC .. "/build-release"