if not dofile 'premake_config.lua' then
	return
end

if _ACTION == "clean" then
    os.rmdir("build")
    return
elseif not _ACTION then
	return
end

solution "XWord"
    configurations(get_configurations())

    -- Output for premake4 (vs projects / makefiles, etc)
    location ("build/" .. _ACTION)

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
        libdirs { "bin/Release" }
        if not _OPTIONS["disable-lua"] then
            libdirs { "lib/Release" }
        end

    configuration "Debug"
        defines { "DEBUG", "_DEBUG" }
        flags { "Symbols" }
        libdirs { "bin/Debug" }
        if not _OPTIONS["disable-lua"] then
            libdirs { "lib/Debug" }
        end

    -- --------------------------------------------------------------------
    -- Platform-specific
    -- --------------------------------------------------------------------
    configuration "windows"
        defines { "WIN32", "_WINDOWS" }
        prebuildcommands {
            [[if not exist "..\..\bin\$(ConfigurationName)" mkdir "..\..\bin\$(ConfigurationName)"]],
            [[if not exist "..\..\bin\$(ConfigurationName)\scripts" mklink /j "..\..\bin\$(ConfigurationName)\scripts" "..\..\scripts"]],
            [[if not exist "..\..\bin\$(ConfigurationName)\images" mklink /j "..\..\bin\$(ConfigurationName)\images" "..\..\images"]],
        }

    configuration "macosx"
        buildoptions { "-mmacosx-version-min=10.7", "-stdlib=libc++" }
        linkoptions  { "-mmacosx-version-min=10.7", "-stdlib=libc++", "-L/usr/local/lib" }

    -- ------------------------------------------------------------------------
    -- General
    -- ------------------------------------------------------------------------

    configuration {}

    include "src" -- the XWord premake file
    include "puz" -- the puzzle library
    include "yajl"
    if not _OPTIONS["disable-lua"] then
        include "lua" -- lua libraries
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
    	-- Put shared libs in XWord.app/Contents/Frameworks
        if get_key(p, "kind") == "SharedLib" then
            project(p.name)
                -- Set the output to the app bundle
                configuration { "macosx", "Debug" }
                    targetdir "bin/Debug/XWord.app/Contents/Frameworks"
                configuration { "macosx", "Release" }
                    targetdir "bin/Release/XWord.app/Contents/Frameworks"
                configuration { "macosx" }
                -- Set the install name
                linkoptions{ "-install_name @executable_path/../Frameworks/lib"..(get_key(p, "targetname") or p.name)..".dylib" }
        elseif get_key(p, "kind") == "WindowedApp" then
            project(p.name)
                if not _OPTIONS["disable-lua"] then
                    -- Requirement for 64-bit OS X applications linking against LuaJIT.
                    -- See http://luajit.org/install.html
                    configuration { "macosx" }
                        linkoptions { "-pagezero_size 10000 -image_base 100000000" }
                end
        end
    end
end

