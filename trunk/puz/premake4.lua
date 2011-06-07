project "puz"
    -- --------------------------------------------------------------------
    -- General
    -- --------------------------------------------------------------------
    kind "SharedLib"
    language "C++"
    files {
        "**.hpp", -- ** means all subdirs
        "**.cpp",
        "**.c",
        "**.h",
    }

    includedirs {
        -- This allows #include "Puzzle.hpp" from the formats subdirs.
        -- The alternative would be #include "../../Puzzle.hpp", which is a pain
        ".",
        "../yajl/build/yajl-2.0.2/include"
    }

    links { "yajl" }

    configuration "windows"
        defines {
            "_USRDLL",
            "YAJL_SHARED",
            "PUZ_API=__declspec(dllexport)",
        }
        -- zlib
        includedirs { "../zlib/include" }
        libdirs { "../zlib/lib" }
        links { "zdll" }

    configuration "linux"
        defines { [[PUZ_API=""]] }
        links { "dl", "zlib" }

    -- Disable some warnings
    configuration "vs*"
        buildoptions {
            "/wd4251", -- DLL Exports
            "/wd4275", -- DLL Exports
        }

    -- Postbuild: copy zlib1.dll to XWord directory

    configuration { "windows", "Debug" }
       postbuildcommands { [[copy "..\..\zlib\zlib1.dll" ..\..\bin\Debug /Y]] }

    configuration { "windows", "Release" }
       postbuildcommands { [[copy "..\..\zlib\zlib1.dll" ..\..\bin\Release /Y]] }
