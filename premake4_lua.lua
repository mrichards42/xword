-- Equivalent of __name__ == '__main__'
if not solution() then -- solution() returns the currently active solution
solution "lua"
    configurations { "Debug", "Release" }
end

    project "lua"
        kind "SharedLib"
        language "C"

        -- Output
        targetname "lua5.1"
        configuration "Debug"   targetdir "bin/Debug"
        configuration "Release" targetdir "bin/Release"
        configuration {}

        -- Source files
        files {
            "lua/lua/src/lapi.*",
            "lua/lua/src/lcode.*",
            "lua/lua/src/ldebug.*",
            "lua/lua/src/ldo.*",
            "lua/lua/src/ldump.*",
            "lua/lua/src/lfunc.*",
            "lua/lua/src/lgc.*",
            "lua/lua/src/llex.*",
            "lua/lua/src/lmem.*",
            "lua/lua/src/lobject.*",
            "lua/lua/src/lopcodes.*",
            "lua/lua/src/lparser.*",
            "lua/lua/src/lstate.*",
            "lua/lua/src/lstring.*",
            "lua/lua/src/ltable.*",
            "lua/lua/src/ltm.*",
            "lua/lua/src/lundump.*",
            "lua/lua/src/lvm.*",
            "lua/lua/src/lzio.*",
            "lua/lua/src/lauxlib.*",
            "lua/lua/src/lbaselib.*",
            "lua/lua/src/ldblib.*",
            "lua/lua/src/liolib.*",
            "lua/lua/src/lmathlib.*",
            "lua/lua/src/loslib.*",
            "lua/lua/src/ltablib.*",
            "lua/lua/src/lstrlib.*",
            "lua/lua/src/loadlib.*",
            "lua/lua/src/linit.*",
        }

        includedirs { "lua/lua", "lua/lua/include" }

        links { "winmm" }

        defines { "_USRDLL", "DLL_EXPORTS", "LUA_BUILD_AS_DLL" }

        -- --------------------------------------------------------------------
        -- Platform-specific
        -- --------------------------------------------------------------------
        configuration "windows"
            defines { "WIN32", "_WINDOWS", "_CRT_SECURE_NO_WARNINGS" }

        -- --------------------------------------------------------------------
        -- Configurations
        -- --------------------------------------------------------------------
        configuration "Release"
            defines { "NDEBUG" }
            flags { "Optimize" }

        configuration "Debug"
            defines { "DEBUG", "_DEBUG", "LUA_USE_APICHECK" }
            flags { "Symbols" }
