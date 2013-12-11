local LIBCURL = "$(LIBCURL)"

project "luacurl"
    kind "SharedLib"
    language "C"

    -- This is the C half of luacurl.  The lua half calls require 'c-luacurl' to
    -- initialize the library.
    targetname "c-luacurl"

    files { "src/*.h", "src/*.c" }

    -- Common lua stuff
    dofile "../premake4_lualib.lua"

    -- Platform-specific
    configuration "windows"
        includedirs { "../../deps/curl/include" }
        links { "wldap32", "winmm", "ws2_32" } -- For curl
        libdirs { "../../deps/curl/lib" }
        links { "libcurl_imp" } -- for luacurl

    configuration "linux or macosx"
        links { "curl" }

    -- Postbuild: copy libcurl.dll to XWord directory

    configuration { "windows", "Debug" }
       postbuildcommands { [[copy "..\..\deps\curl\lib\*.dll" ..\..\bin\Debug /Y]] }

    configuration { "windows", "Release" }
       postbuildcommands { [[copy "..\..\deps\curl\lib\*.dll" ..\..\bin\Release /Y]] }
