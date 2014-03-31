project "lxp"
    kind "SharedLib"
    language "C"

    -- Output
    targetname "lxp"

    -- Source files
    files {
        "src/lxplib.c",
        "src/lxplib.h",
    }

    -- Common lua stuff
    dofile "../premake4_lualib.lua"

    -- Platform-specific
    configuration "windows"
        defines { "_CRT_SECURE_NO_WARNINGS" }
        includedirs { "../../deps/expat/include" }
        libdirs { "../../deps/expat/lib" }
        links { "libexpat" }

    configuration "linux or macosx"
        links { "expat" }

    -- Postbuild: copy libexpat.dll to XWord directory

    configuration { "windows", "Debug" }
       postbuildcommands { [[copy "..\..\deps\expat\lib\libexpat.dll" ..\..\bin\Debug /Y]] }

    configuration { "windows", "Release" }
       postbuildcommands { [[copy "..\..\deps\expat\lib\libexpat.dll" ..\..\bin\Release /Y]] }
