project "expat"
    kind "SharedLib"
    language "C"
    files {
        -- Source files
        "lib/xmlparse.c",
        "lib/xmlrole.c",
        "lib/xmltok.c",
        "lib/xmltok_impl.c",
        "lib/xmltok_ns.c",
        -- Headers files
        "lib/expat.h",
        "lib/expat_external.h",
        -- Export definitions
        "lib/libexpat.def",
    }

    -- Output
    targetname "libexpat"

    configuration "windows"
        defines {
            "_USRDLL",
            "COMPILED_FROM_DSP", -- Tell expat that this is windows
        }

    configuration "linux"
        links { "dl" }
