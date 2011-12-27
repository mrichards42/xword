dofile '../premake_config.lua'

project "XWord"
    -- --------------------------------------------------------------------
    -- General
    -- --------------------------------------------------------------------
    kind "WindowedApp"
    language "C++"

    files
    {
        "**.hpp", "**.cpp", "**.h",
    }

    if not _OPTIONS["disable-lua"] then
        defines { "XWORD_USE_LUA" }
    else
    	excludes { "xwordbind/*" }
    end

    configuration "windows"
        -- Use WinMain() instead of main() for windows apps
        flags { "WinMain" }

    -- --------------------------------------------------------------------
    -- wxWidgets
    -- --------------------------------------------------------------------
    configuration "linux"
        -- These link options ensure that the wxWidgets libraries are
        -- linked in the correct order under linux.
        if not _OPTIONS["disable-lua"] then
            linkoptions {
                "-lwxbindxrc",
                "-lwxbindxml",
                "-lwxbindnet",
                "-lwxbindhtml",
                "-lwxbindaui",
                "-lwxbindadv",
                "-lwxbindcore",
                "-lwxbindbase",
                "-lwxlua",
                "-llua5.1",
            }
        end

    dofile "../premake4_wxdefs.lua"
    dofile "../premake4_wxlibs.lua"

    -- --------------------------------------------------------------------
    -- puz
    -- --------------------------------------------------------------------
    includedirs { "../" }
    links { "puz" }
    configuration "windows"
        defines {
            "PUZ_API=__declspec(dllimport)",
            "LUAPUZ_API=__declspec(dllimport)",
        }

    configuration "linux"
        defines { [[PUZ_API=""]] }
        links { "dl" }
    
    configuration "macosx"
    	defines {
    		[[PUZ_API=\"\"]],
            [[LUAPUZ_API=\"\"]],
            "USE_FILE32API" -- for minizip
    	}

    -- Disable some warnings
    configuration "vs*"
        buildoptions {
            "/wd4800", -- implicit conversion to bool
            "/wd4251", -- DLL Exports
        }
    -- --------------------------------------------------------------------
    -- wxLua
    -- --------------------------------------------------------------------
    configuration {}
	if not _OPTIONS["disable-lua"] then
        includedirs {
            "../lua",
            "../lua/lua/include",
            "../lua/wxbind/setup",
        }
    end

    if  not _OPTIONS["disable-lua"] then
        links {
            "lua",
            "wxlua",
            "wxbindbase",
            "wxbindcore",
            "wxbindadv",
            "wxbindaui",
            "wxbindhtml",
            "wxbindnet",
            "wxbindxml",
            "wxbindxrc",
            "luapuz",
        }
    end

    -- --------------------------------------------------------------------
    -- Resource files
    -- --------------------------------------------------------------------
    configuration "windows"
        files { "**.rc" }
        resincludedirs { ".." }

