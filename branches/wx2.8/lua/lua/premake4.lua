project "lua"
    kind "SharedLib"
    language "C"

    -- Output
    targetname "lua5.1"

    -- Source files
    files {
        "src/lapi.*",
        "src/lcode.*",
        "src/ldebug.*",
        "src/ldo.*",
        "src/ldump.*",
        "src/lfunc.*",
        "src/lgc.*",
        "src/llex.*",
        "src/lmem.*",
        "src/lobject.*",
        "src/lopcodes.*",
        "src/lparser.*",
        "src/lstate.*",
        "src/lstring.*",
        "src/ltable.*",
        "src/ltm.*",
        "src/lundump.*",
        "src/lvm.*",
        "src/lzio.*",
        "src/lauxlib.*",
        "src/lbaselib.*",
        "src/ldblib.*",
        "src/liolib.*",
        "src/lmathlib.*",
        "src/loslib.*",
        "src/ltablib.*",
        "src/lstrlib.*",
        "src/loadlib.*",
        "src/linit.*",
    }

    includedirs { ".", "include" }

    -- --------------------------------------------------------------------
    -- Platform-specific
    -- --------------------------------------------------------------------
    configuration "windows"
        defines {
            "LUA_BUILD_AS_DLL",
            "_USRDLL",
            "DLL_EXPORTS",
            "_CRT_SECURE_NO_WARNINGS",
        }
        links "winmm"

    configuration "linux"
        defines "LUA_USE_LINUX"
        links "dl"

    configuration "macosx"
        defines {
            "LUA_USE_MACOSX",
            "LUA_USE_DLOPEN"
        }

    -- --------------------------------------------------------------------
    -- Debug / Release
    -- --------------------------------------------------------------------
    configuration "Debug"
        defines { "LUA_USE_APICHECK" }
