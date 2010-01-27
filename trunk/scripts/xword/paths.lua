-- XWord paths

local stdpaths = wx.wxStandardPaths.Get()
local exedir = wx.wxPathOnly(stdpaths.ExecutablePath)

-- See if we have a portable_mode_enabled file in the executable directory.
xword.isPortable = wx.wxFileExists(exedir..'/portable_mode_enabled')

if xword.isPortable then
    xword.configdir  = exedir..'/config'
    xword.imagesdir  = exedir..'/images'
    xword.scriptsdir = exedir..'/scripts'
else
    xword.configdir  = stdpaths.UserDataDir..'/config'
    xword.imagesdir  = stdpaths.ResourcesDir..'/images'
    xword.scriptsdir = stdpaths.UserDataDir..'/scripts'
end

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
    if dirname ~= '' then dirname = dirname .. '/' end
    local pkgname = dirname:gsub('[\\/]', '.')

    -- iterate all files and directories
    for script in lfs.dir(xword.scriptsdir..'/'..dirname) do
        if script:sub(1,1) ~= '.' and script ~= 'init.lua' then
            -- Is this a file or a directory?
            local attr = lfs.attributes(xword.scriptsdir..'/'..dirname..script)
            if attr.mode == 'file' and script:sub(-4) == '.lua' then
                print ('file:'..script)
                require(pkgname..script:sub(1,-5))
            elseif packages and attr.mode == 'directory' then
                print ('dir:'..script)
                -- Is there an init file in the directory?
                if lfs.attributes(xword.scriptsdir..'/'..dirname..script..'/init.lua') then
                    require(pkgname..script)
                end
            end
        end
    end
end
