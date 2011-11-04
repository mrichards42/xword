-- wxWidgets defines and include directories
dofile 'premake_config.lua'

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

if os.is("macosx") then
configuration { "macosx", "Debug" }
    buildoptions{ trim(os.outputof(WXMAC_BUILD_DEBUG .. "/wx-config --cxxflags")) }

configuration { "macosx", "Release" }
    buildoptions{ trim(os.outputof(WXMAC_BUILD_RELEASE .. "/wx-config --cxxflags")) }
end
-- Reset the configuration
configuration {}
