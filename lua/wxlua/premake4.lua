project "wxlua"
    -- --------------------------------------------------------------------
    -- General
    -- --------------------------------------------------------------------
    kind "StaticLib"
    language "C++"
    files { "*", "debug/*", "debugger/*" }
    includedirs {
        "../wxbind/setup",
        DEPS.lua.include,
        "..",
        "."
    }
    libdirs { DEPS.lua.lib }
    links { "lua51" }

    configuration "Debug"   targetdir "../../lib/Debug"
    configuration "Release" targetdir "../../lib/Release"

    -- --------------------------------------------------------------------
    -- Platform-specific
    -- --------------------------------------------------------------------
    configuration "windows"
        defines { "_LIB" }

    -- Disable some warnings
    configuration "vs*"
        buildoptions {
            "/wd4800", -- implicit conversion to bool
            "/wd4390", -- empty control statement
        }

    -- wxWidgets
    dofile "../../premake4_wxdefs.lua"
