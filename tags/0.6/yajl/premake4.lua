project "yajl"
    -- --------------------------------------------------------------------
    -- General
    -- --------------------------------------------------------------------
    kind "SharedLib"
    language "C"
    files {
        "src/*.h",
        "src/*.c",
        "src/api/*.h",
        "src/api/*.c"
    }

    includedirs { "build/yajl-2.0.2/include" }

    targetname "yajl"

    configuration "windows"
        defines {
            "WIN32",
            "_WINDOWS",
            "YAJL_SHARED",
            "YAJL_BUILD"
        }

    configuration "linux"
        defines {
            "YAJL_BUILD",
        }
        links { "dl" }

    -- Disable some warnings
    configuration "vs*"
        buildoptions {
            "/wd4996",
            "/wd4255",
            "/wd4130",
            "/wd4100",
            "/wd4711"
        }

    configuration "linux"
        buildoptions {
            "-std=c99",
            "-pedantic",
            "-Wpointer-arith",
            "-Wno-format-y2k",
            "-Wstrict-prototypes",
            "-Wmissing-declarations",
            "-Wnested-externs",
            "-Wextra",
            "-Wundef",
            "-Wwrite-strings",
            "-Wold-style-definition",
            "-Wredundant-decls",
            "-Wno-unused-parameter",
            "-Wno-sign-compare",
            "-Wmissing-prototypes",
            "-O2",
            "-Wuninitialized"
        }
