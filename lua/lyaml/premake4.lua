project "lyaml"
    kind "SharedLib"
    language "C"

    -- Output
    targetname "c-yaml"

    -- Source files
    files {
        "ext/yaml/*.c",
        "ext/yaml/*.h",
    }

    sysincludedirs { "../../yaml/include" }

    links { "yaml" }

    -- Common lua stuff
    dofile "../premake4_lualib.lua"

    defines {
        [[VERSION=""]],
    }
