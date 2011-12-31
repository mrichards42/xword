-- wxWidgets defines and include directories
configuration {}
    defines { "UNICODE", "_UNICODE", }

configuration "Debug"
    defines { "__WXDEBUG__" }

if os.is("windows") then

configuration "windows"
    defines { "__WXMSW__" }
    includedirs { _OPTIONS["wx-prefix"].."/include" }

    configuration { "windows", "Release" }
        includedirs { _OPTIONS["wx-prefix"].."/lib/vc_lib/mswu" }

    configuration { "windows", "Debug" }
        includedirs { _OPTIONS["wx-prefix"].."/lib/vc_lib/mswud" }

elseif os.is("linux") then

configuration { "linux", "Debug" }
    buildoptions(string.format("`%s --debug --unicode --static --cxxflags`",
    							_OPTIONS["wx-config-debug"]))

configuration { "linux", "Release" }
    buildoptions(string.format("`%s --release --unicode --static --cxxflags`",
    							_OPTIONS["wx-config-release"]))

elseif os.is("macosx") then

configuration { "macosx", "Debug" }
    buildoptions(wx_config("--debug --unicode --static --cxxflags"))

configuration { "macosx", "Release" }
    buildoptions(wx_config("--release --unicode --static --cxxflags"))

end

-- Reset the configuration
configuration {}
