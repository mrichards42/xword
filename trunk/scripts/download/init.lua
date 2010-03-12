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

-- Take the "return " out of serialized strings
local global_serialize = serialize
local function serialize(var)
    return string.gsub(global_serialize(var), 'return ', '')
end

local function savevariable(f, varname, var)
    f:write(varname.." = "..serialize(var))
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
    -- Find the string names for curl options
    require 'curl'
    local optnames = {}
    for opt, val in pairs(curl) do
        if opt:sub(1,3) == 'OPT' then
            assert(not optnames[val]) -- No doubles allowed
            optnames[val] = opt
        end
    end

    -- Save the sources table
    f = assert(io.open(xword.configdir .. '/download/sources.lua', 'wb'))
    f:write("-- Download sources\n")
    f:write("\n")
    f:write('download.sources =\n')
    f:write('{\n')
        for _, source in ipairs(download.sources) do
            f:write('    {\n')
                -- Save normal options
                savevariable(f, '        display', source.display)
                f:write(",\n")
                savevariable(f, '        url', source.url)
                f:write(",\n")
                savevariable(f, '        filename', source.filename)
                f:write(",\n")
                savevariable(f, '        days', source.days)
                f:write(",\n")
                -- Save format if it exists
                if source.format then
                    savevariable(f, '        format', source.format)
                    f:write(",\n")
                end
                -- Save curlopts if they exist
                if source.curlopts then
                    f:write("        curlopts = \n")
                    f:write("        {\n")
                    for opt, val in pairs(source.curlopts) do
                        f:write("            [curl."..optnames[opt].."] = "
                                ..serialize(val))
                        f:write(",\n")
                    end
                    f:write("        },\n")
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
