dofile 'premake_config.lua'

if _ACTION == "clean" then
    os.rmdir("build")
end

solution "XWord"
    configurations { "Release", "Debug" }
    configuration "macoxs"
        platforms "x32"
    configuration {}

    -- Output for premake4 (vs projects / makefiles, etc)
    location ("build/" .. (_ACTION or ''))

    -- Output for compiled files
    configuration "Debug"
        targetdir "bin/Debug"

    configuration "Release"
        targetdir "bin/Release"

    -- --------------------------------------------------------------------
    -- Debug / Release
    -- --------------------------------------------------------------------
    configuration "Release"
        defines { "NDEBUG" }
        flags { "Optimize" }
        libdirs { "bin/Release", "lib/Release" }

    configuration "Debug"
        defines { "DEBUG", "_DEBUG" }
        flags { "Symbols" }
        libdirs { "bin/Debug", "lib/Debug" }

    -- --------------------------------------------------------------------
    -- Platform-specific
    -- --------------------------------------------------------------------
    configuration "windows"
        defines { "WIN32", "_WINDOWS" }

    -- ------------------------------------------------------------------------
    -- General
    -- ------------------------------------------------------------------------

    configuration {}

    include "src" -- the XWord premake file
    include "puz" -- the puzzle library
    include "yajl"
    if not _OPTIONS["disable-lua"] then
        include "lua" -- lua libraries
        include "expat"
    end


-- Mac stuff
if os.is("macosx") then
	-- Find a value in project.blocks
	local function get_key(p, k)
		if p[k] then return p[k] end
		for _, block in ipairs(p.blocks) do
			if block[k] then
				return block[k]
			end
		end
	end
	
	for _, p in pairs(solution().projects) do
		if get_key(p, "kind") == "SharedLib" then
			print(p.name)
			project(p.name)
				-- Set the output to the app bundle
				configuration "Debug"
					targetdir "bin/Debug/XWord.app/Contents/Frameworks"
				configuration "Release"
					targetdir "bin/Release/XWord.app/Contents/Frameworks"

				-- Set the install name
				linkoptions{ "-install_name @executable_path/../Frameworks/lib"..(get_key(p, "targetname") or p.name)..".dylib" }
		end
	end
end