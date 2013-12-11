project "luatask"
    kind "SharedLib"
    language "C"

    -- Output
    -- This is the C half of luatask.  The lua half calls require 'c-task' to
    -- initialize the library.
    targetname "c-task"

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

    configuration "linux or macosx"
        defines {
            "LUATASK_PTHREAD_STACK_SIZE=2097152/16",
            [[LUATASK_API=\"\"]]
        }
        links { "pthread" }
