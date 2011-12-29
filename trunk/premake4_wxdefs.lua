-- wxWidgets defines and include directories
configuration {}
    defines { "UNICODE", "_UNICODE", }

configuration "Debug"
    defines { "__WXDEBUG__" }

configuration "windows"
    defines { "__WXMSW__" }
    includedirs { _OPTIONS["wx-prefix"].."/include" }

    configuration { "windows", "Release" }
        includedirs { _OPTIONS["wx-prefix"].."/lib/vc_lib/mswu" }

    configuration { "windows", "Debug" }
        includedirs { _OPTIONS["wx-prefix"].."/lib/vc_lib/mswud" }

configuration { "linux", "Debug" }
    buildoptions "`wx-config --debug --unicode --static --cxxflags`"

configuration { "linux", "Release" }
    buildoptions "`wx-config --release --unicode --static --cxxflags`"

if os.is("macosx") then
configuration { "macosx", "Debug" }
    buildoptions(cmd(WXMAC_BUILD_DEBUG .. "/wx-config --cxxflags"))

configuration { "macosx", "Release" }
    buildoptions(cmd(WXMAC_BUILD_RELEASE .. "/wx-config --cxxflags"))
end
-- Reset the configuration
configuration {}
