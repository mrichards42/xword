-- ============================================================================
-- Puzzle download
--     A quite complicated XWord add-on that downloads puzzles from a variety
--     of sources.

--     This add-on is implemented as a package and contains many files and even
--     a sub-project (layouts).
--     Add-ons with multiple files must be placed into their own directory.
--     An init.lua script is the only script that will be called to initialize
--     the add-on.
-- ============================================================================


-- Create the download table
download = {}

-- Add the download modules
require 'download.defs'
require 'download.dialog'

-- ----------------------------------------------------------------------------
-- Configuration
-- ----------------------------------------------------------------------------
require 'download.defaultconfig'
pcall(dofile, xword.configdir..'/download/config.lua') -- Use default config if this fails

require 'download.defaultsources'
pcall(dofile, xword.configdir..'/download/sources.lua') -- Use default sources if this fails

require 'serialize'
require 'io'
local function savevariable(f, varname, var)
    local str = string.gsub(serialize(var), 'return ', varname.." = ")
    f:write(str)
end

local function saveconfig()
    f = assert(io.open(xword.configdir .. '/download/config.lua', 'wb'))
    f:write("-- Download package configuration\n")
    f:write("\n")
    f:write("-- Download output folder (created if it doesn't exist\n")
    savevariable(f, 'download.localfolder', download.localfolder)
    f:write("\n\n")
    f:write("-- strftime format for column headers in the download dialog\n")
    savevariable(f, 'download.dateformat', download.dateformat)
    f:write("\n\n")
    f:close()
end

local function savesources()
    f = assert(io.open(xword.configdir .. '/download/sources.lua', 'wb'))
    f:write("-- Download sources\n")
    f:write("\n")
    f:write('download.sources =\n')
    f:write('{\n')
        for _, source in ipairs(download.sources) do
            f:write('    {\n')
                savevariable(f, '        display', source.display)
                f:write(",\n")
                savevariable(f, '        url', source.url)
                f:write(",\n")
                savevariable(f, '        filename', source.filename)
                f:write(",\n")
                savevariable(f, '        days', source.days)
                f:write(",\n")
                if source.format then
                    savevariable(f, '        format', source.format)
                    f:write(",\n")
                end
            f:write('    },\n\n')
        end
    f:write('}\n')
    f:close()
end

-- ----------------------------------------------------------------------------
-- init
-- ----------------------------------------------------------------------------

local function init()
    -- Add the download menu item
    local menuItem = xword.frame:AddMenuItem({'Tools'}, 'Download puzzles',
        function(evt)
            local dlg = download.GetDialog()
            dlg:Show()
        end
    )

    -- Save configuration and sources when the frame closes
    xword.frame:Connect(wx.wxEVT_CLOSE_WINDOW,
        function(evt)
            saveconfig()
            savesources()
            evt:Skip()
        end
    )
end

init()
