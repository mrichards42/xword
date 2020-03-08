-- ===========================================================================
-- Options
-- ===========================================================================
newoption {
    trigger = "disable-lua",
    description = "Disable support for lua packages",
}

newoption {
	trigger     = "wx-prefix",
    value       = "path",
    description = "Custom wxWidgets directory"
}

if not os.istarget("windows") then
newoption {
	trigger = "wx-config",
	value = "path",
	description = "Custom wx-config to use"
}

newoption {
	trigger = "wx-config-debug",
	value = "path",
	description = "Custom wx-config to use for debug builds."
}

newoption {
	trigger = "wx-config-release",
	value = "path",
	description = "Custom wx-config to use for release builds."
}

end


-- End of options
if not _ACTION or _ACTION == "clean" then
	return false
end


-- Return output of the given command stripped of extra whitespace
function cmd(command_text)
    local text = os.outputof(command_text)
    -- split text
    local t = {}
    for word in text:gmatch("%S+") do
        table.insert(t, word)
    end
    return table.concat(t, " ")
end

-- ===========================================================================
-- Ensure that we have enough _OPTIONS, and fill in the blanks
-- ===========================================================================

-- Search for a wx-config in a list of directories (and all subdirs)
-- return a table of the best matches (i.e. highest version numbers)
-- { debug =   { path = "/path/to/wx-config", version = "version.string" },
--   release = { path = "/path/to/wx-config", version = "version.string" } }
-- If a debug or release config doesn't exist, it will be set to nil
local function get_wx_config(directories, use_system_wide_config)
	local t = {}
	function is_higher_version(v2, v1)
		a,b,c = v1:match("(%d+)%.(%d+)%.(%d+)")
		x,y,z = v2:match("(%d+)%.(%d+)%.(%d+)")
		if not a or not x then return false end
		return x * 10000 + y * 100 + z > a * 10000 + b * 100 + c
	end
	-- Check the wx-config in this path and add it to t if it's newer than
	-- the current version
	function add_wx_config(config)
		local version = cmd(config .. ' --version')
		local build = cmd(config .. ' --list')
		local debug = build:match("debug")
		local release = build:match("release")
		if not (version and debug or release) then return end
		-- Check against previous versions
		function add_if_newer(build)
			if t[build] then
				if is_higher_version(version, t[build].version) then
					t[build].path = config
					t[build].version = version
				end
			elseif is_higher_version(version, "0.0.0") then
				t[build] = { path = config, version = version }
			end
		end
		if debug then add_if_newer("debug") end
		if release then add_if_newer("release") end
	end
	-- Check a directory for wx-config
	-- Print a dot every 1000 directories for long-running operations
	local counter = 0
	function test_config(directory)
		counter = counter + 1
		if counter % 1000 == 0 then io.write('.') io.flush() end
		-- Look for wx-config in this directory
		local config = path.join(directory, 'wx-config')
		if os.isfile(config) then
			add_wx_config(config)
		end
		-- Look in subdirs
		for _, subdir in ipairs(os.matchdirs(path.join(directory, "*"))) do
			test_config(path.join(directory, subdir))
		end
	end
	-- Check the given directories
	if type(directories) == "string" then
		test_config(directories)
	else
		for _, d in ipairs(directories) do
			test_config(d)
		end
	end
	-- Also try the global wx-config
	if use_system_wide_config and cmd('whereis wx-config') ~= "" then
		add_wx_config('wx-config')
	end
	if counter > 1000 then print() end
	return t
end


-- Check _OPTIONS to see if we have a wx-config; if we don't, search for it.
if os.istarget("windows") then
	if not _OPTIONS["wx-prefix"] then
		local wxwin = os.getenv("WXWIN")
		_OPTIONS["wx-prefix"] = "$(WXWIN)"
		print("Using wxWidgets installation at $(WXWIN):")
		if wxwin then
			print("    "..wxwin)
		else
			print("    (environmental variable doesn't exist)")
		end
	end
elseif not (_OPTIONS["wx-config-debug"] and _OPTIONS["wx-config-release"]) then
	-- If we got a wx-config option, our work is done
	if _OPTIONS["wx-config"] then
		_OPTIONS["wx-config-debug"] = _OPTIONS["wx-config-debug"] or _OPTIONS["wx-config"]
		_OPTIONS["wx-config-debug"] = _OPTIONS["wx-config-release"] or _OPTIONS["wx-config"]
	-- Otherwise we'll have to search for wx-config
	else
		local configs
		local wxwin = os.getenv("WXWIN")
		-- First try the user-supplied wx-prefix
		if _OPTIONS["wx-prefix"] then
			print("Searching for wx-config with")
			print("    --wx-prefix=" .. _OPTIONS["wx-prefix"])
			configs = get_wx_config(_OPTIONS["wx-prefix"])
		-- Then try the $WXWIN environmental variable
		elseif wxwin then
			print("Searching for wx-config at $WXWIN (" .. wxwin .. ")")
			configs = get_wx_config(wxwin)
		-- Then search for wxWidgets under whatever paths could make sense
		else
			local paths = {
				'/Developer',
			}
			-- Compile a list of folders beginning with wx
			local config_paths = {}
			for _, directory in ipairs(paths) do
				for _, p in ipairs(os.matchdirs(path.join(directory, "wx*"))) do
					table.insert(config_paths, path.join(directory, p))
				end
			end
			-- Look for wx-config somewhere within these folders
			print("Searching for wx-config in these locations:")
			for _, p in ipairs(config_paths) do
				print("    " .. p)
			end
			configs = get_wx_config(config_paths, true) -- Also use system-wide
		end
		-- Check out our results
		if not _OPTIONS["wx-config-debug"] then
			if configs.debug then
				_OPTIONS["wx-config-debug"] = configs.debug.path
			else
				print("Could not find a wx-config for a debug build.")
			end
		end
		if not _OPTIONS["wx-config-release"] then
			if configs.release then
				_OPTIONS["wx-config-release"] = configs.release.path
			else
				print("Could not find a wx-config for a release build.")
			end
		end
	end
end

-- ===========================================================================
-- Public functions
-- ===========================================================================

-- Safely execute and return the value of a wx-config command
-- cache the results so error messages aren't printed a bunch of times
local wx_config_cache = {}
function wx_config(options)
	if wx_config_cache[options] then return wx_config_cache[options] end
	local result = (function()
		local config
		if options:match("debug") then
			config = _OPTIONS["wx-config-debug"]
		else
			config = _OPTIONS["wx-config-release"]
		end
		-- Don't break if we don't have this configuration
		if not config then return "" end
		return cmd(config .. " " .. options)
	end)()
	wx_config_cache[options] = result
	return result
end

-- Return a list of valid configurations (based on which wx-configs we found)
function get_configurations()
	if not os.istarget("windows") then
		-- Check wx-config
		if not (_OPTIONS["wx-config-debug"] or _OPTIONS["wx-config-release"]) then
			print("Unable to find wx-config on this system.")
			error()
		end
		function check_build(build)
			local config = _OPTIONS["wx-config-" .. build]
			if config then
				print("Using wx-config for " .. build .. " builds:")
				local version = cmd(config .. ' --version')
				print("    Path: " ..config)
				print("    Version: " .. version)
				table.insert(configs, build)
			else
				print("No wx-config exists for a " .. build .. " build.\n" ..
					  "*** Output will not contain a " .. build .. " configuration. ***")
			end
		end
		configs = {}
		check_build("debug")
		check_build("release")
	end
	return { "Debug", "Release" }
end

-- Dependency paths

-- Make paths absolute and adjust dll path
local function paths(paths)
    if paths.dll then
        paths.debugdll = paths.lib .. '/debug/' .. paths.dll
        paths.dll = paths.lib .. '/' .. paths.dll
    end
    for k,v in pairs(paths) do
        paths[k] = path.getabsolute(v)
    end
    if paths.dll then
        paths.dll = path.getrelative('build/release', paths.dll):gsub('/','\\')
        paths.debugdll = path.getrelative('build/release', paths.debugdll):gsub('/','\\')
        paths.copyrelease = [[copy "]] .. paths.dll .. [[" ..\..\bin\Release /Y]]
        -- Try to copy from lib/debug first, then lib if that fails
        paths.copydebug = ([[if exist "%s" (copy "%s" ..\..\bin\Debug /Y) else (copy "%s" ..\..\bin\Debug /Y)]]):format(paths.debugdll, paths.debugdll, paths.dll)
    end
    return paths
end
DEPS = {
    lua = paths{include="deps/luajit/include", lib="deps/luajit/lib/" .. os.get(), dll="lua51.dll"},
    zlib = paths{include="deps/zlib/include", lib="deps/zlib/lib", dll="zlib1.dll"},
    expat = paths{include="deps/expat/include", lib="deps/expat/lib", dll="libexpat.dll"},
    curl = paths{include="deps/curl/include", lib="deps/curl/lib", dll="*.dll"},
}

return true
