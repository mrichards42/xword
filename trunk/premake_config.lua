-- General options
newoption {
    trigger = "disable-lua",
    description = "Disable support for lua packages",
}


-- Windows options
WXWIN = "$(WXWIN)"

-- Mac options
WXMAC = "/Developer/wxMac-2.8.12"
WXMAC_BUILD_DEBUG = WXMAC .. "/build-release"
WXMAC_BUILD_RELEASE = WXMAC .. "/build-release"


-- Don't mess with anything below this
function cmd(command_text)
	local text = os.outputof(command_text)
	-- split text
	local t = {}
	for word in text:gmatch("%S+") do
		table.insert(t, word)
	end
	return table.concat(t, " ")
end
