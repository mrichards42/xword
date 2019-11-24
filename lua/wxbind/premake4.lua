-- Template for each project
local function wxbindproject(name)
    project("wxbind"..name)
        kind "StaticLib"
        language "C++"
        files { "src/wx"..name.."*", "include/wx"..name.."*" }

        defines { "_LIB" }
        includedirs { "setup", DEPS.lua.include, "..",  }
        files { "src/dummy.cpp" }

        configuration "Debug"   targetdir "../../lib/Debug"
        configuration "Release" targetdir "../../lib/Release"

        -- Disable some warnings
        configuration "vs*"
            buildoptions {
                "/wd4996", -- deprecation
            }
            linkoptions {
                "/ignore:4006", -- object already defined
                "/ignore:4221", -- no public symbols
            }

        dofile "../../premake4_wxdefs.lua"
end

-- ------------------------------------------------------------------------
-- wxbind projects
-- ------------------------------------------------------------------------

wxbindproject "xrc"
    links { "wxbindadv", "wxbindhtml", "wxbindxml" }

wxbindproject "xml"
    links { "wxbindbase" }

wxbindproject "net"
    links { "wxbindbase" }

wxbindproject "html"
    links { "wxbindcore" }

wxbindproject "aui"
    links { "wxbindcore" }

wxbindproject "adv"
    links { "wxbindcore" }

wxbindproject "core"
    links { "wxbindbase" }

wxbindproject "base"
    files { "include/wxbinddefs.h" }
    links { "wxlua" }
