local wxWidgets = '$(WXWIN)'

-- Equivalent of __name__ == '__main__'
if not solution() then -- solution() returns the currently active solution
solution "wxlua"
    configurations { "Debug", "Release" }
end

    project "wxlua"
        -- --------------------------------------------------------------------
        -- General
        -- --------------------------------------------------------------------
        kind "StaticLib"
        language "C++"
        files { "lua/wxlua/src/*", "lua/wxlua/include/*" }

        defines { "UNICODE", "_UNICODE", }
        includedirs { "lua/wxbind/setup", "lua/lua/include", "lua",  }

        -- Output directories
        configuration "Debug"   targetdir "lib/Debug"
        configuration "Release" targetdir "lib/Release"
        configuration {}

        -- --------------------------------------------------------------------
        -- Build configurations
        -- --------------------------------------------------------------------
        configuration "Release"
            defines { "NDEBUG" }
            flags { "Optimize" }

        configuration "Debug"
            defines { "DEBUG", "_DEBUG", "__WXDEBUG__" }
            flags { "Symbols" }


        -- --------------------------------------------------------------------
        -- Platform-specific
        -- --------------------------------------------------------------------
        configuration "windows"
            defines { "WIN32", "_WINDOWS", "_LIB" }

        -- Disable some warnings
        configuration "vs*"
            buildoptions {
                "/wd4800", -- implicit conversion to bool
                "/wd4390", -- empty control statement
            }

        -- --------------------------------------------------------------------
        -- wxWidgets
        -- --------------------------------------------------------------------
        configuration {}

        -- Platform-specific
        configuration "windows"
            includedirs { wxWidgets.."/include" }
            defines { "__WXMSW__" }

            configuration { "windows", "Release" }
                includedirs { wxWidgets .. "/lib/vc_lib/mswu" }

            configuration { "windows", "Debug" }
                includedirs { wxWidgets .. "/lib/vc_lib/mswud" }

        configuration "linux"
            buildoptions "`wx-config --cxxflags`"
            linkoptions "`wx-config --libs`"
