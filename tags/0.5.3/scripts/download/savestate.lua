
require 'serialize'
local path = require 'pl.path'
local dir = require 'pl.dir'

-- Take the "return " out of serialized strings
local global_serialize = serialize
local function serialize(var)
    return string.gsub(global_serialize(var), 'return ', '')
end

local function savevariable(f, varname, var)
    f:write(varname.." = "..serialize(var))
end

local function saveconfig()
    f = assert(io.open(path.join(xword.configdir, 'download', 'config.lua'), 'wb'))
    f:write("require 'date'\n")
    f:write("\n")
    f:write("-- Download package configuration\n")
    f:write("\n")
    f:write("-- Download output folder (created if it doesn't exist\n")
    savevariable(f, 'download.localfolder', download.localfolder)
    f:write("\n\n")
    f:write("-- strftime format for column headers in the download dialog\n")
    savevariable(f, 'download.dateformat', download.dateformat)
    f:write("\n\n")
    f:write("-- strftime format for puzzle files\n")
    savevariable(f, 'download.puzformat', download.puzformat)
    f:write("\n\n")
    f:write("-- Dialog size\n")
    savevariable(f, 'download.dlgsize', download.dlgsize)
    f:write("\n\n")
    f:write("-- Dialog filter state\n")
    if not download.filterState then
        download.filterState = {}
    end
    download.filterState.date = nil
    savevariable(f, 'download.filterState', download.filterState)
    f:write("\n\n")
    f:close()
end

local function savesources()
    -- Find the string names for curl options
    require 'luacurl'
    local optnames = {}
    for opt, val in pairs(curl) do
        if opt:sub(1,3) == 'OPT' then
            assert(not optnames[val]) -- No doubles allowed
            optnames[val] = opt
        end
    end

    -- Save the sources table
    f = assert(io.open(path.join(xword.configdir, 'download', 'sources.lua'), 'wb'))
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
                savevariable(f, '        prefix', source.prefix)
                f:write(",\n")
                savevariable(f, '        ext', source.ext)
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

-- The puzzle index / cache will be saved along the way (it's a sqlite database)


function download.savestate()
    dir.makepath(path.join(xword.configdir, 'download'))
    saveconfig()
    savesources()
end