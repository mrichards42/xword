project "luayajl"
    kind "SharedLib"
    language "C"

    -- Output
    targetname "luayajl"

    -- Source files
    files {
        "lua_yajl.c",
    }

    links { "yajl" }

    -- Common lua stuff
    dofile "../premake4_lualib.lua"

    -- Platform-specific
    configuration "windows"
        defines { "_CRT_SECURE_NO_WARNINGS" }
        includedirs { "../../yajl/build/yajl-2.0.2/include" }
