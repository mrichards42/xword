-- wxWidgets defines and include directories

local WXWIN = "$(WXWIN)"

configuration {}
    defines { "UNICODE", "_UNICODE", }

configuration "Debug"
    defines { "__WXDEBUG__" }

configuration "windows"
    defines { "__WXMSW__" }
    includedirs { WXWIN.."/include" }

    configuration { "windows", "Release" }
        includedirs { WXWIN.."/lib/vc_lib/mswu" }

    configuration { "windows", "Debug" }
        includedirs { WXWIN.."/lib/vc_lib/mswud" }

configuration { "linux", "Debug" }
    buildoptions "`wx-config --debug --unicode --static --cxxflags`"

configuration { "linux", "Release" }
    buildoptions "`wx-config --release --unicode --static --cxxflags`"

-- Reset the configuration
configuration {}
