project "luatask"
    kind "SharedLib"
    language "C"

    -- Output
    targetname "task"

    -- Source files
    files { "src/*.c", }

    dofile "../premake4_lualib.lua"

    -- --------------------------------------------------------------------
    -- Platform-specific
    -- --------------------------------------------------------------------
    configuration "windows"
        defines {
            "LUATASK_API=__declspec(dllexport)",
            "TASK_EXPORTS",
            "NATV_WIN32",
        }

    configuration "linux"
        defines {
            "LUATASK_PTHREAD_STACK_SIZE=2097152/16",
            [[LUATASK_API=""]]
        }
        links { "pthread" }
