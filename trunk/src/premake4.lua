project "XWord"
    -- --------------------------------------------------------------------
    -- General
    -- --------------------------------------------------------------------
    kind "WindowedApp"
    language "C++"
    files { "**.hpp", "**.cpp", "**.h" }

    defines { "XWORD_USE_LUA" }

    configuration "windows"
        -- Use WinMain() instead of main() for windows apps
        flags { "WinMain" }

    -- --------------------------------------------------------------------
    -- wxWidgets
    -- --------------------------------------------------------------------
    configuration "linux"
        -- These link options ensure that the wxWidgets libraries are
        -- linked in the correct order under linux.
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

    dofile "../premake4_wxdefs.lua"
    dofile "../premake4_wxlibs.lua"

    -- --------------------------------------------------------------------
    -- wxLua
    -- --------------------------------------------------------------------
    configuration {}
    includedirs {
        "../lua",
        "../lua/lua/include",
        "../lua/wxbind/setup",
    }

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
    }

    -- --------------------------------------------------------------------
    -- Resource files
    -- --------------------------------------------------------------------
    configuration "windows"
        files { "**.rc" }
        resincludedirs { ".." }
