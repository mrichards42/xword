-- Equivalent of __name__ == '__main__'
if not solution() then -- solution() returns the currently active solution
solution "luatask"
    configurations { "Debug", "Release" }
end

    project "luatask"
        kind "SharedLib"
        language "C"

        -- Output
        targetname "task"
        configuration "Debug"   targetdir "bin/Debug/scripts/libs"
        configuration "Release" targetdir "bin/Release/scripts/libs"
        configuration {}

        -- Source files
        files {
            "lua/luatask/src/*.c",
        }

        includedirs { "lua/lua", "lua/lua/include" }

        defines { "LUA_LIB" }

        links { "lua" }

        -- --------------------------------------------------------------------
        -- Platform-specific
        -- --------------------------------------------------------------------
        configuration "windows"
            defines {
                "LUATASK_API=__declspec(dllexport)",
                "TASK_EXPORTS", "NATV_WIN32",
                "WIN32", "_WINDOWS",
                "_USRDLL", "LUA_BUILD_AS_DLL",
            }

        configuration "linux"
            defines { "LUA_USE_LINUX",
                       "LUATASK_PTHREAD_STACK_SIZE=2097152/16",
                       "LUATASK_API" }
            links { "dl", "pthread" }

        -- --------------------------------------------------------------------
        -- Configurations
        -- --------------------------------------------------------------------
        configuration "Release"
            defines { "NDEBUG" }
            flags { "Optimize" }

        configuration "Debug"
            defines { "DEBUG", "_DEBUG", "LUA_USE_APICHECK" }
            flags { "Symbols" }
