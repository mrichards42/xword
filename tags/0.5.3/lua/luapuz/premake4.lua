project "luapuz"
    kind "SharedLib"
    language "C++"
    files {
        "*.cpp",
        "*.hpp",
        "bind/*.cpp",
        "bind/*.hpp",
    }

    -- Common lua stuff
    dofile "../premake4_lualib.lua"

    configuration "Debug"   targetdir "../../bin/Debug"
    configuration "Release" targetdir "../../bin/Release"
    configuration {}

    includedirs { "../.." }
    links { "puz" }

    configuration "windows"
        defines {
            "PUZ_API=__declspec(dllimport)",
            "LUAPUZ_API=__declspec(dllexport)",
        }

    -- Disable some warnings
    configuration "vs*"
        buildoptions {
            "/wd4800", -- implicit conversion to bool
            "/wd4244", -- conversion: possible loss of data
            "/wd4251", -- class needs dll-interface
        }


    configuration "linux"
        defines { [[PUZ_API=""]] }
        links { "dl" }

