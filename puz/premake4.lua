project "puz"
    -- --------------------------------------------------------------------
    -- General
    -- --------------------------------------------------------------------
    kind "SharedLib"
    language "C++"
    files {
        "Checksummer.*",
        "exceptions.*",
        "Grid.*",
        "LoadPuz.*",
        "Puzzle.*",
        "SavePuz.*",
        "Scrambler.*",
        "Square.*",
        "streamwraper.*",
        "util.*",
    }

    configuration "windows"
        defines {
            "_USRDLL",
            "PUZ_API=__declspec(dllexport)",
        }

    configuration "linux"
        defines { [[PUZ_API=""]] }
        links { "dl" }

    -- Disable some warnings
    configuration "vs*"
        buildoptions {
            "/wd4251", -- DLL Exports
        }