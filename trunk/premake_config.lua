-- ===========================================================================
-- Options
-- ===========================================================================
newoption {
    trigger = "disable-lua",
    description = "Disable support for lua packages",
}

if not os.is("linux") then
newoption {
	trigger     = "wx-prefix",
    value       = "path",
    description = "Custom wxWidgets directory; defaults by system\n" ..
                  "     windows: \t $(WXWIN)\n" ..
                  "     mac: \t /Developer/wxMac-[latest version]\n" ..
                  "     linux: \t unused (system-wide wx-config is used)"
}
end

if not os.is("windows") then
newoption {
	trigger = "wx-config",
	value = "path",
	description = 
		"Custom wx-config to use (instead of the system-wide " ..
								"wx-config); defaults by system:\n" ..
	    "     windows: \t unused (use --wx-prefix instead)\n" ..
	    "     mac: \t wx-config found in build[-release/-debug] directory\n" ..
	    "     linux: system-wide wx-config"
}
end


-- End of options
if not _ACTION then
	return false
end

if not _OPTIONS["wx-prefix"] and not os.is("linux") then
	print("Searching for wxWidgets installation . . .")
	local wxwin = os.getenv("WXWIN")
	if os.is("windows") then
		_OPTIONS["wx-prefix"] = "$(WXWIN)"
		print("Using wxWidgets installation set through $(WXWIN):")
		if wxwin then
			print("    "..wxwin)
		else
			print("    (environmental variable doesn't exist)")
		end
	elseif os.is("macosx") then
		if wxwin then
			print("Using wxWidgets installation set through $(WXWIN):")
			print("    "..wxwin)
			_OPTIONS["wx-prefix"] = wxwin
		else
			-- Search for wxWidgets in various locations
			local paths = {
				'/Developer',
			}
			function has_wx_config(p)
				return os.pathsearch('wx-config', path.join(p, 'build-debug'), path.join(p, 'build-release'))
			end
			local highest_version = 0
			for _, directory in ipairs(paths) do
				for _, p in ipairs(os.matchdirs(path.join(directory, "wx*"))) do
					local v1, v2, v3 = p:match(path.join(directory, "wx.*(%d+)%.(%d+)%.(%d+)"))
					if v1 then
						version = tonumber(v1) * 10000 + tonumber(v2) * 100 + tonumber(v3)
						if version > highest_version and has_wx_config(p) then
							highest_version = version
							_OPTIONS["wx-prefix"] = p
						end
					elseif highest_version == 0 and has_wx_config(p) then
						_OPTIONS["wx-prefix"] = p
					end
				end
			end
			if _OPTIONS["wx-prefix"] then
				print("Using wxWidgets installation found at:")
				print("    ".._OPTIONS["wx-prefix"])
				WXMAC_BUILD_DEBUG = _OPTIONS["wx-prefix"] .. "/build-debug"
				WXMAC_BUILD_RELEASE = _OPTIONS["wx-prefix"] .. "/build-release"
			else
				error("Could not find wxWidgets installation.")
				return false
			end
		end
	end
end


if not _OPTIONS["wx-config"] then
	_OPTIONS["wx-config"] = "wx-config"
end

function cmd(command_text)
    local text = os.outputof(command_text)
    -- split text
    local t = {}
    for word in text:gmatch("%S+") do
        table.insert(t, word)
    end
    return table.concat(t, " ")
end

return true
