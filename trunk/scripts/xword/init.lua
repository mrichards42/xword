-- ============================================================================
-- XWord additions
--     This package is loaded by XWord to initialize lua extensions and
--     add-ons
-- ============================================================================


-- ============================================================================
-- Paths
--
-- This section is placed in the init file instead of a separate paths.lua
-- script because they are used to set lua's search paths and initialize the
-- rest of XWord's lua extensions and add-ons.
--
-- Functions
--     xword.makeDirs(filename)
--     io.open(filename, mode)
--     xword.requiredir(dirname, includePackages)
-- ============================================================================

local function join(...)
    return table.concat(arg, string.char(wx.wxFileName.GetPathSeparator()))
end

-- Figure out the standard paths used by Xword
local stdpaths = wx.wxStandardPaths.Get()
local exedir = wx.wxPathOnly(stdpaths.ExecutablePath)

if xword.IsPortable() then
    xword.configdir  = join(exedir, 'config')
    xword.imagesdir  = join(exedir, 'images')
    xword.scriptsdir = join(exedir, 'scripts')
else
    xword.configdir  = join(stdpaths.UserDataDir, 'config')
    xword.imagesdir  = join(stdpaths.ResourcesDir, 'images')
    xword.scriptsdir = join(stdpaths.UserDataDir, 'scripts')
end

-- Set the search paths
package.path = table.concat({
    join(xword.scriptsdir, '?.lua'),
    join(xword.scriptsdir, '?', 'init.lua'),
    join(xword.scriptsdir, 'libs', '?.lua'),
    join(xword.scriptsdir, 'libs', '?', '.lua'),
}, ';')

local ext
local osId = wx.wxPlatformInfo.Get():GetOperatingSystemId()
if bit.band(osId, wx.wxOS_WINDOWS) ~= 0 then
    ext = 'dll'
elseif bit.band(osId, wx.wxOS_MAC) ~= 0 then
    ext = 'dylib'
else
    ext = 'so'
end

package.cpath = table.concat({
    join(xword.scriptsdir, 'libs', '?.'..ext),
    join(xword.scriptsdir, 'libs', '?51.'..ext),
}, ';')



-- ----------------------------------------------------------------------------
-- Functions
-- ----------------------------------------------------------------------------

-- Ensure that the filename is reachable by making intermediate directories
function xword.makeDirs(filename)
    local fn = wx.wxFileName(filename)
    if not fn:DirExists() then
        fn:Mkdir(4095, -- default permissions
                 wx.wxPATH_MKDIR_FULL) -- Make intermediate dirs
    end
end

-- Redefine the lua io.open function to create directories that do not
-- exists when opening a file for writing.
local io_open = io.open
function io.open(filename, mode)
    if mode:find('w') then xword.makeDirs(filename) end
    return io_open(filename, mode)
end

-- Require all files in a given directory that are not named init.lua
-- Optionally, also attempt to require subdirs as packages (by subdir name)
require 'lfs'
function xword.requiredir(dirname, packages)
    local dirname = assert(dirname)
    -- Lua packages use the naming convention directory.file
    if dirname ~= '' then dirname = dirname .. '/' end
    local pkgname = dirname:gsub('[\\/]', '.')

    dirname = xword.scriptsdir..'/'..dirname

    -- iterate all files and directories
    for script in lfs.dir(dirname) do
        if script:sub(1,1) ~= '.' and script ~= 'init.lua' then
            -- Is this a file or a directory?
            local attr = lfs.attributes(dirname..script)
            if attr.mode == 'file' and script:sub(-4) == '.lua' then
                print ('file:'..script)
                require(pkgname..script:sub(1,-5))
            elseif packages and attr.mode == 'directory' then
                print ('dir:'..script)
                -- Is there an init file in the directory?
                if lfs.attributes(dirname..script..'/init.lua') then
                    require(pkgname..script)
                end
            end
        end
    end
end

require 'xword.utils'
require 'xword.messages'
require 'xword.utf-8'

-- Require the scripts directory, including subdirs
xword.requiredir('', true)
