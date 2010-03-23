project "lfs"
    kind "SharedLib"
    language "C"

    -- Output
    targetname "lfs"

    -- Source files
    files {
        "src/lfs.c",
        "src/lfs.h",
    }

    -- Common lua stuff
    dofile "../premake4_lualib.lua"

    -- --------------------------------------------------------------------
    -- Platform-specific
    -- --------------------------------------------------------------------
    configuration "windows"
        defines {
            "DLL_EXPORTS",
            "_CRT_SECURE_NO_WARNINGS",
        }

    -- Disable some warnings
    configuration "vs*"
        buildoptions {
            "/wd4996", -- deprecation
            "/wd4244", -- conversion data loss
        }
