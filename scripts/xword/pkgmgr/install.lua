--[[
    install.lua
    Contains package unstallation and unistallation functions
]]


local P = xword.pkgmgr

local join = require 'xword.pkgmgr.join'
local serialize = require 'serialize'

local function _sanitize_directory_name(name)
    local t = {}
    for s in name:gmatch('([^/\\%.]+)') do
        table.insert(t, s)
    end
    return join(unpack(t))
end


-- ===========================================================================
-- Uninstall
-- ===========================================================================

-- Uninstall a package
local function _do_uninstall(packagename, delete_config)
    -- Don't allow uninstalling libraries for now
    local kind = 'script'

    -- Try to unload the package
    require 'xword.pkgmgr.load'
    if not P.unload_package(packagename) then
        return false
    end

    -- Delete all the files
    local pkgdir = _sanitize_directory_name(packagename)
    local rmtree = require 'pl.dir'.rmtree
    rmtree(join(xword.userdatadir, pkgdir))
    if delete_config then
        rmtree(join(xword.configdir, pkgdir))
    end
    rmtree(join(xword.imagesdir, pkgdir))
    if kind == "library" then
        rmtree(join(xword.scriptsdir, 'libs', pkgdir))
        os.remove(join(xword.scriptsdir, 'libs', pkgdir..'.lua'))
        os.remove(join(xword.scriptsdir, 'libs', pkgdir..'.dll'))
        os.remove(join(xword.scriptsdir, 'libs', pkgdir..'.so'))
    else
        rmtree(join(xword.scriptsdir, pkgdir))
    end

    return true
end

-- Uninstall a package given its package name
-- If the package was enabled and loaded, try to unload it first.
-- If we can't unload the package, add it to the pending_uninstall.lua file
function P.uninstall_package(packagename, delete_config)
    if not _do_uninstall(packagename, delete_config) then
        -- Can't unload the package; we'll have to wait for a restart
        local filename = join(xword.userdatadir, 'pending_uninstall.lua')
        local uninstall = serialize.loadfile(filename) or {}
        uninstall[packagename] = (delete_config == true)
        serialize.dump(uninstall, filename)
        return false
    end

    return true
end


-- ===========================================================================
-- Install
-- ===========================================================================

-- Create a wxFileSystem with CopyFile and CopyDirectory functions based off
-- our existing extension with a ReadFile function.
local function _new_fs(archive)
    local fs = xword.NewFs(archive, join)

    function fs:CopyFile(inFile, outFile)
        local f = self:OpenFile(inFile)
        if not f then return false end
        local out = wx.wxFileOutputStream(outFile)
        if out:IsOk() then
            f:GetStream():Read(out)
        end
        local ret = out:IsOk()
        out:Close()
        out:delete()
        f:delete()
        return ret
    end

    function fs:CopyDirectory(inDir, outDir)
        local base = self.archive..inDir
        local wildcard = join(base, '*')

        -- Check for any files
        -- Don't log an error if the directory is nonexistant
        local l = wx.wxLogNull()
        local dir_exists = self:FindFirst(wildcard)
        l:delete()
        if dir_exists == '' then
            return false
        end

        -- Make the output directory
        if not wx.wxDirExists(outDir) then
            wx.wxFileName.Mkdir(outDir, 511, wx.wxPATH_MKDIR_FULL)
        end

        -- List files
        local file = self:FindFirst(wildcard, wx.wxFILE)
        while file ~= '' do
            file = file:sub(#base+2)
            self:CopyFile(join(base, file), join(outDir, file))
            file = self:FindNext()
        end

        -- List dirs
        -- We must finish the FindFirst / FindNext cycle here before starting
        -- a new one by calling CopyDirectory.
        local dirs = {}
        local dir = self:FindFirst(wildcard, wx.wxDIR)
        while dir ~= '' do
            dir = dir:sub(#base+2)
            table.insert(dirs, dir)
            dir = self:FindNext()
        end

        for _, dir in ipairs(dirs) do
            self:CopyDirectory(join(inDir, dir), join(outDir, dir))
        end

        return true
    end

    return fs
end

--[[
    Install a package given a filename.
    This can be either an archive or a folder.

    Package structure is expected to be as follows:
     * - required
    [] - optional
        ./
            *info.lua*
            scripts/[package/] ...
            images/[package/] ...
            config/[package/] ...
            data/[package/] ...

    info.lua is a script that returns a package table in standard format with
    these required fields:
    {
        name = "Name",
        version = "1.0",
        kind = "script",  *must* be "script" or "library"
        packagename = "package",
        [...]
    }

    Files will be placed in the following folders:
        scripts/[package/] -> xword.scriptsdir/package/
        images/[package/]  -> xword.imagesdir/package/
        config/[package/]  -> xword.configdir/package/
        data/[package/]    -> xword.userdatadir/package/

    If kind is "library", files in scripts/ will be placed
    in xword.scriptsdir/libs/

    If install_package could not uninstall the package, return false.
    If there is a problem installing the package, return nil, message.
    Otherwise return the package name
]]
function P.install_package(filename)
    local fs, err = _new_fs(filename)
    if not fs then
        return nil, err
    end

    -- Look for an info.lua file
    local function load_info()
        local info = fs:ReadFile(join(fs.archive, 'info.lua'))
        if not info then
            return nil, "Required file info.lua is missing."
        end
        local pkg, err = serialize.loadstring(info)
        if not pkg then
            return nil, "info.lua file is unreadable."
        end
        if not pkg.name and pkg.version and pkg.requires and pkg.packagename then
            return nil, "info.lua file is invalid or incomplete."
        end
        return pkg
    end

    local pkg, err = load_info()
    if not pkg then
        fs:delete()
        return nil, err
    end

    -- Make sure that the package doesn't require a newer version of xword
    assert(not P.is_newer(pkg.requires, xword.version),
           "This package requires a newer version of XWord.")

    -- Try to uninstall the package (but leave the config dir alone, and don't
    -- uninstall on restart)
    if not _do_uninstall(pkg.packagename, false) then
        -- Can't uninstall the package; we'll have to wait for a restart
        local pending_filename = join(xword.userdatadir, 'pending_install.lua')
        local install = serialize.loadfile(pending_filename) or {}
        install[pkg.packagename] = filename
        serialize.dump(install, pending_filename)
        fs:delete()
        return false
    end

    local pkgdir = _sanitize_directory_name(pkg.packagename)

    -- Make the package directory


    -- Look for special directories:
    -- images  -> xword.imagesdir
    -- scripts -> xword.scriptsdir
    -- data    -> xword.userdatadir
    -- config  -> xword.configdir

    -- Files in images, data, and config are all placed in a subfolder that
    -- is packagename.
    -- If this is a library, copy the scripts folder verbatim into the libs
    -- directory.  Otherwise, add a subfolder as with other directories

    local xword_dirs = {
        images   = xword.imagesdir,
        data     = xword.userdatadir,
        config   = xword.configdir,
    }

    if pkg.kind == "library" then
        fs:CopyDirectory('scripts', join(xword.scriptsdir, 'libs'))
    else
        xword_dirs.scripts = xword.scriptsdir
    end

    -- Try directory/package; if that fails, try directory/
    -- e.g. scripts/database/*.* or scripts/*.*
    for name, dir in pairs(xword_dirs) do
        if not fs:CopyDirectory(join(name, pkgdir), join(dir, pkgdir)) then
            fs:CopyDirectory(name, join(dir, pkgdir))
        end
    end

    fs:delete()
    return pkg.packagename
end
