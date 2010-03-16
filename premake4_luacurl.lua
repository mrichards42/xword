-- Equivalent of __name__ == '__main__'
if not solution() then -- solution() returns the currently active solution
solution "luacurl"
    configurations { "Debug", "Release" }
end

    project "luacurl"
        kind "SharedLib"
        language "C"
        files { "lua/luacurl/src/*.h", "lua/luacurl/src/*.c" }

        -- Lua defines, includes, and libs
        defines { "LUA_BUILD_AS_DLL", "LUA_LIB" }
        includedirs { "lua/lua/include" }
        links { "lua" }

        -- Output directories
        configuration "Debug"
            libdirs "bin/Debug"
            targetdir "bin/Debug/scripts/libs"

            -- Copy libcurl.dll to XWord directory
            configuration { "windows", "Debug" }
               postbuildcommands { [[copy "%LIBCURL%\lib\DLL-Release\libcurl.dll" ..\..\bin\Debug /Y]] }

        configuration "Release"
            libdirs "bin/Release"
            targetdir "bin/Release/scripts/libs"

            -- Copy libcurl.dll to XWord directory
            configuration { "windows", "Release" }
               postbuildcommands { [[copy "%LIBCURL%\lib\DLL-Release\libcurl.dll" ..\..\bin\Release /Y]] }

        -- common cURL defines, includes, and libs
        defines {}

        -- Platform-specific
        configuration "windows"
            -- libraries needed by cURL
            links { "wldap32", "winmm", "ws2_32" }
            includedirs { "$(LIBCURL)/include" }
            libdirs { "$(LIBCURL)/lib/DLL-Release" }
            links { "libcurl_imp" }

        configuration "linux"
            links { "curl" }


        -- Configurations
        configuration "Release"
            defines { "NDEBUG" }
            flags { "Optimize" }

        configuration "Debug"
            defines { "DEBUG" }
            flags { "Symbols" }
