project "lsqlite3"
    kind "SharedLib"
    language "C"
    files { "lsqlite3.c", "sqlite3.c", "sqlite3.h" }

    -- Common lua stuff
    dofile "../premake4_lualib.lua"

    includedirs { "." }
    -- Platform-specific
    configuration "windows"
        defines {
            "_CRT_SECURE_NO_WARNINGS",
        }

        -- Disable some warnings
        configuration "vs*"
            buildoptions {
                "/wd4244", -- conversion
            }