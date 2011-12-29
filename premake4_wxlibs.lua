-- wxWidgets links
configuration "windows"
    libdirs { _OPTIONS["wx-prefix"].."/lib/vc_lib" }
    links {
        "winmm",
        "comctl32",
        "rpcrt4",
        "wsock32",
        "odbc32",
        "oleacc",
    }

    configuration { "windows", "Release" }
        links {
            "wxmsw28u_xrc",
            "wxmsw28u_html",
            "wxmsw28u_gl",
            "wxmsw28u_aui",
            "wxmsw28u_media",
            "wxmsw28u_adv",
            "wxbase28u_net",
            "wxbase28u_xml",
            "wxmsw28u_core",
            "wxbase28u",
            "wxtiff",
            "wxjpeg",
            "wxpng",
            "wxzlib",
            "wxregexu",
            "wxexpat",
        }

    configuration { "windows", "Debug" }
        links {
            "wxmsw28ud_xrc",
            "wxmsw28ud_html",
            "wxmsw28ud_gl",
            "wxmsw28ud_aui",
            "wxmsw28ud_media",
            "wxmsw28ud_adv",
            "wxbase28ud_net",
            "wxbase28ud_xml",
            "wxmsw28ud_core",
            "wxbase28ud",
            "wxtiffd",
            "wxjpegd",
            "wxpngd",
            "wxzlibd",
            "wxregexud",
            "wxexpatd",
        }

configuration { "linux", "Debug" }
    linkoptions  "`wx-config --debug --unicode --static --libs`"

configuration { "linux", "Release" }
    linkoptions  "`wx-config --release --unicode --static --libs`"

if os.is("macosx") then
configuration { "macosx", "Debug" }
    linkoptions(cmd(WXMAC_BUILD_DEBUG .. "/wx-config --libs"))

configuration { "macosx", "Release" }
    linkoptions(cmd(WXMAC_BUILD_RELEASE .. "/wx-config --libs"))
end

configuration {}
