local wxWidgets = '$(WXWIN)'


solution "XWord"
    configurations { "Debug", "Release" }

    location ("build/".._ACTION)

    -- ------------------------------------------------------------------------
    -- General
    -- ------------------------------------------------------------------------
    -- Copy the images and scripts directories to the build directory

    configuration {}


    -- ------------------------------------------------------------------------
    -- XWord
    -- ------------------------------------------------------------------------
    project "XWord"
        links {
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
        -- General
        -- --------------------------------------------------------------------
        kind "WindowedApp"
        language "C++"
        files { "src/**.hpp", "src/**.cpp", "src/**.h" }

        defines { "UNICODE", "_UNICODE", "XWORD_USE_LUA" }

        configuration "Debug"   targetdir "bin/Debug"
        configuration "Release" targetdir "bin/Release"

        configuration "windows"
            defines { "WIN32", "_WINDOWS" }
            -- Use WinMain() instead of main() for windows apps
            flags { "WinMain" }

        -- --------------------------------------------------------------------
        -- Configurations
        -- --------------------------------------------------------------------
        configuration "Release"
            defines { "NDEBUG" }
            flags { "Optimize" }

        configuration "Debug"
            defines { "DEBUG", "_DEBUG", "__WXDEBUG__" }
            flags { "Symbols" }

        -- --------------------------------------------------------------------
        -- wxWidgets
        -- --------------------------------------------------------------------
        configuration {}
        includedirs { wxWidgets.."/include" }

        -- Platform-specific
        configuration "windows"
            defines { "__WXMSW__" }
            libdirs { wxWidgets.."/lib/vc_lib" }
            links {
                "winmm",
                "comctl32",
                "rpcrt4",
                "wsock32",
                "odbc32",
                "oleacc",
            }

            configuration { "windows", "Release" }
                includedirs { wxWidgets .. "/lib/vc_lib/mswu" }
                links {
                    "wxmsw28u_xrc",
                    "wxmsw28u_html",
                    "wxmsw28u_gl",
                    "wxmsw28u_aui",
                    "wxmsw28u_media",
                    "wxmsw28u_adv",
                    "wxbase28u_net",
                    "wxbase28u_xml",
                    "wxmsw28u_core",
                    "wxbase28u",
                    "wxtiff",
                    "wxjpeg",
                    "wxpng",
                    "wxzlib",
                    "wxregexu",
                    "wxexpat",
                }

            configuration { "windows", "Debug" }
                includedirs { wxWidgets .. "/lib/vc_lib/mswud" }
                links {
                    "wxmsw28ud_xrc",
                    "wxmsw28ud_html",
                    "wxmsw28ud_gl",
                    "wxmsw28ud_aui",
                    "wxmsw28ud_media",
                    "wxmsw28ud_adv",
                    "wxbase28ud_net",
                    "wxbase28ud_xml",
                    "wxmsw28ud_core",
                    "wxbase28ud",
                    "wxtiffd",
                    "wxjpegd",
                    "wxpngd",
                    "wxzlibd",
                    "wxregexud",
                    "wxexpatd",
                }


        -- --------------------------------------------------------------------
        -- wxLua
        -- --------------------------------------------------------------------
        configuration {}
        includedirs {
            "lua",
            "lua/lua/include",
            "lua/wxbind/setup",
        }

        -- --------------------------------------------------------------------
        -- Resource files
        -- --------------------------------------------------------------------
        configuration "windows"
            files { "src/**.rc" }
            resincludedirs { "." }



    -- ------------------------------------------------------------------------
    -- Lua
    -- ------------------------------------------------------------------------
    configuration {}

    -- Dependencies
    dofile "premake4_lua.lua"

    dofile "premake4_wxlua.lua"
        links { "lua" }

    dofile "premake4_wxbind.lua"

    project "wxbindbase"
        links { "wxlua" }
