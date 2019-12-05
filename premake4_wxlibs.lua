-- wxWidgets links
if os.istarget("windows") then

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

    -- Determine the version by looking at the directory
    local v1, v2 = _OPTIONS["wx-prefix"]:match("wxWidgets%-(%d+)%.(%d%+)")
    local version
    if v1 and v2 then
        version = v1 .. v2
    end
    if not version then
        -- Determine the version by looking at the libraries
        local libs = os.matchfiles(_OPTIONS["wx-prefix"].."/lib/vc_lib/wxmsw*.lib")
        if libs then
            version = libs[1]:match("wxmsw(%d%d)")
        end
    end
    if version then
        print("Using wxWidgets version " .. version)
    else
        print("Unable to determine wxWidgets version.")
        error()
    end

    configuration { "windows", "Release" }
        links {
            "wxmsw" .. version .. "u_xrc",
            "wxmsw" .. version .. "u_html",
            "wxmsw" .. version .. "u_gl",
            "wxmsw" .. version .. "u_aui",
            "wxmsw" .. version .. "u_media",
            "wxmsw" .. version .. "u_adv",
            "wxbase" .. version .. "u_net",
            "wxbase" .. version .. "u_xml",
            "wxmsw" .. version .. "u_core",
            "wxbase" .. version .. "u",
            "wxtiff",
            "wxjpeg",
            "wxpng",
            "wxzlib",
            "wxregexu",
            "wxexpat",
        }

    configuration { "windows", "Debug" }
        links {
            "wxmsw" .. version .. "ud_xrc",
            "wxmsw" .. version .. "ud_html",
            "wxmsw" .. version .. "ud_gl",
            "wxmsw" .. version .. "ud_aui",
            "wxmsw" .. version .. "ud_media",
            "wxmsw" .. version .. "ud_adv",
            "wxbase" .. version .. "ud_net",
            "wxbase" .. version .. "ud_xml",
            "wxmsw" .. version .. "ud_core",
            "wxbase" .. version .. "ud",
            "wxtiffd",
            "wxjpegd",
            "wxpngd",
            "wxzlibd",
            "wxregexud",
            "wxexpatd",
        }

elseif os.istarget("linux") then

configuration { "linux", "Debug" }
    linkoptions(string.format("`%s --debug --unicode --static --libs`",
                                _OPTIONS["wx-config-debug"]))

configuration { "linux", "Release" }
    linkoptions(string.format("`%s --release --unicode --static --libs`",
                                _OPTIONS["wx-config-release"]))

elseif os.istarget("macosx") then

configuration { "macosx", "Debug" }
    linkoptions(wx_config("--debug --unicode --static --libs std aui"))

configuration { "macosx", "Release" }
    linkoptions(wx_config("--unicode --static --libs std aui"))

end

configuration {}
