USE_LUA = true

solution "XWord"
    configurations { "Release", "Debug" }

    -- Output for premake4 (vs projects / makefiles, etc)
    location ("build/".._ACTION)

    -- Output for compiled files
    configuration { "Debug" }
        targetdir "bin/Debug"

    configuration { "Release" }
        targetdir "bin/Release"

    -- --------------------------------------------------------------------
    -- Debug / Release
    -- --------------------------------------------------------------------
    configuration "Release"
        defines { "NDEBUG" }
        flags { "Optimize" }
        libdirs { "bin/Release", "lib/Release" }

    configuration "Debug"
        defines { "DEBUG", "_DEBUG" }
        flags { "Symbols" }
        libdirs { "bin/Debug", "lib/Debug" }

    -- --------------------------------------------------------------------
    -- Platform-specific
    -- --------------------------------------------------------------------
    configuration "windows"
        defines { "WIN32", "_WINDOWS" }

    -- ------------------------------------------------------------------------
    -- General
    -- ------------------------------------------------------------------------

    configuration {}

    include "src" -- the XWord premake file
    include "puz" -- the puzzle library
    if USE_LUA then
        include "lua" -- lua libraries
    end