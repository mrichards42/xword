project "puz"
    -- --------------------------------------------------------------------
    -- General
    -- --------------------------------------------------------------------
    kind "SharedLib"
    language "C++"
    files {
        "*.hpp",
        "*.cpp"
    }

    includedirs { "../expat/lib" }

    links { "expat" }

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
            "/wd4275", -- DLL Exports
        }