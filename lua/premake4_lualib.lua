-- Common setup for lua shared libs

configuration "Debug"
    targetdir "../bin/Debug/scripts/libs"
configuration "Release"
    targetdir "../bin/Release/scripts/libs"
configuration {}

includedirs { "lua", "lua/include" }
defines { "LUA_LIB" }
links { "lua" }

configuration "windows"
    defines {
        "_USRDLL", "LUA_BUILD_AS_DLL",
    }
    links { "winmm" }

configuration "linux"
    defines { "LUA_USE_LINUX" }
    links { "dl" }

configuration "Debug"
    defines { "LUA_USE_APICHECK" }

configuration {}