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
        includedirs { DEPS.curl.include }
        links { "wldap32", "ws2_32", "crypt32", "normaliz" } -- For curl
        libdirs { DEPS.curl.lib }
        links { "libcurl" } -- for luacurl

    configuration "linux or macosx"
        links { "curl" }

    -- Postbuild: copy libcurl.dll to XWord directory

    configuration { "windows", "Debug" }
       postbuildcommands { DEPS.curl.copydebug }

    configuration { "windows", "Release" }
       postbuildcommands { DEPS.curl.copyrelease }
