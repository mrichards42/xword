-- Equivalent of __name__ == '__main__'
if not solution() then -- solution() returns the currently active solution
solution "lfs"
    configurations { "Debug", "Release" }
end

    project "lfs"
        kind "SharedLib"
        language "C"

        -- Output
        targetname "lfs"
        configuration "Debug"   targetdir "bin/Debug/scripts/libs"
        configuration "Release" targetdir "bin/Release/scripts/libs"
        configuration {}

        -- Source files
        files {
            "lua/lfs/src/lfs.c",
            "lua/lfs/src/lfs.h",
        }

        includedirs { "lua/lua", "lua/lua/include" }

        defines { "LUA_LIB" }

        links { "lua" }

        -- --------------------------------------------------------------------
        -- Platform-specific
        -- --------------------------------------------------------------------
        configuration "windows"
            defines { "_WIN32", "_WINDOWS", "_CRT_SECURE_NO_WARNINGS",
                      "_USRDLL", "DLL_EXPORTS", "LUA_BUILD_AS_DLL" }

        configuration "linux"
            defines { "LUA_USE_LINUX" }
            links { "dl" }

        -- --------------------------------------------------------------------
        -- Configurations
        -- --------------------------------------------------------------------
        configuration "Release"
            defines { "NDEBUG" }
            flags { "Optimize" }

        configuration "Debug"
            defines { "DEBUG", "_DEBUG", "LUA_USE_APICHECK" }
            flags { "Symbols" }
