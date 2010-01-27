local wxWidgets = '$(WXWIN)'

-- Template for each project
-- Hide this under a project heading so that it doesn't pollute the "global"
-- solution when we include this file in the XWord project
local function wxbindproject(name)
project("wxbind"..name)
    kind "StaticLib"
    language "C++"
    files { "lua/wxbind/src/wx"..name.."*", "lua/wxbind/include/wx"..name.."*" }

    -- ------------------------------------------------------------------------
    -- Common
    -- ------------------------------------------------------------------------
    defines { "UNICODE", "_UNICODE", "_LIB" }
    includedirs { "lua/wxbind/setup", "lua/lua/include", "lua",  }
    files { "lua/wxbind/src/dummy.cpp" }

    -- Output directories
    configuration "Debug"   targetdir "lib/Debug"
    configuration "Release" targetdir "lib/Release"
    configuration {}

    -- ------------------------------------------------------------------------
    -- Build configurations
    -- ------------------------------------------------------------------------
    configuration "Release"
        defines { "NDEBUG" }
        flags { "Optimize" }

    configuration "Debug"
        defines { "DEBUG", "_DEBUG", "__WXDEBUG__" }
        flags { "Symbols" }

    -- ------------------------------------------------------------------------
    -- Platform-specific
    -- ------------------------------------------------------------------------
    configuration "windows"
        defines { "WIN32", "_WINDOWS" }

    -- Disable some warnings
    configuration "vs*"
        buildoptions {
            "/wd4996", -- deprecation
        }

    -- ------------------------------------------------------------------------
    -- wxWidgets
    -- ------------------------------------------------------------------------
    configuration {}
    includedirs { wxWidgets.."/include" }

    -- Platform-specific
    configuration "windows"
        defines { "__WXMSW__" }

        configuration { "windows", "Release" }
            includedirs { wxWidgets .. "/lib/vc_lib/mswu" }

        configuration { "windows", "Debug" }
            includedirs { wxWidgets .. "/lib/vc_lib/mswud" }

end

-- Equivalent of __name__ == '__main__'
if not solution() then -- solution() returns the currently active solution
solution "wxbind"
    configurations { "Debug", "Release" }
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
        files { "lua/wxbind/include/wxbinddefs.h" }
