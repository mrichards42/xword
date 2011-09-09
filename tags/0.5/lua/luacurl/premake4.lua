local LIBCURL = "$(LIBCURL)"

project "luacurl"
    kind "SharedLib"
    language "C"
    files { "src/*.h", "src/*.c" }

    -- Common lua stuff
    dofile "../premake4_lualib.lua"

    -- Platform-specific
    configuration "windows"
        includedirs { LIBCURL.."/include" }
        links { "wldap32", "winmm", "ws2_32" } -- For curl
        libdirs { LIBCURL.."/lib/DLL-Release" }
        links { "libcurl_imp" } -- for luacurl

    configuration "linux"
        links { "curl" }

    -- Postbuild: copy libcurl.dll to XWord directory

    configuration { "windows", "Debug" }
       postbuildcommands { [[copy "%LIBCURL%\lib\DLL-Release\libcurl.dll" ..\..\bin\Debug /Y]] }

    configuration { "windows", "Release" }
       postbuildcommands { [[copy "%LIBCURL%\lib\DLL-Release\libcurl.dll" ..\..\bin\Release /Y]] }
