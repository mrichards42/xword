USE_LUA = true

project "XWord"
    -- --------------------------------------------------------------------
    -- General
    -- --------------------------------------------------------------------
    kind "WindowedApp"
    language "C++"
    files
    {
        "*.hpp", "*.cpp", "*.h",
        "widgets/*.hpp",   "widgets/*.cpp",   "widgets/*.h",
        "utils/*.hpp",     "utils/*.cpp",     "utils/*.h",
        "dialogs/*.hpp",   "dialogs/*.cpp",   "dialogs/*.h",
        "xwordbind/*.hpp", "xwordbind/*.cpp", "xwordbind/*.h",
    }

    if USE_LUA then
        defines { "XWORD_USE_LUA" }
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
        if USE_LUA then
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
    if USE_LUA then
        includedirs {
            "../lua",
            "../lua/lua/include",
            "../lua/wxbind/setup",
        }
    end

    if USE_LUA then
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
