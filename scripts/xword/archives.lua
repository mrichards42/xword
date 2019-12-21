local endswith = require 'pl.stringx'.endswith

-- Add the given handler for this file type if we don't know how to open it yet
local function _add_handler_if_needed(test, constructor)
    if wx.wxFileSystem.HasHandlerForPath(test) then
        return
    end
    local h = constructor()
    wx.wxFileSystem.AddHandler(h)
    -- wxLua should ungc the handler in this function, but it doesn't.
    -- If lua garbage collects this handler before we are done using it,
    -- we'll crash.
    wxlua.ungcobject(h)
end

-- Create a wxFileSystem with ReadFile function
-- Callers may optionally provide a custom implementation of a path joining
-- function - the default implementation depends on lfs, which is a .dll.
function xword.NewFs(archive, join)
    join = join or require 'pl.path'.join

    -- Make sure we can read archive files
    _add_handler_if_needed('blah.zip#zip:', wx.wxArchiveFSHandler)
    _add_handler_if_needed('blah.gz#gzip:', wx.wxFilterFSHandler)

    -- Add the wxFileSystem handler information to the archive path
    if endswith(archive, '.zip') then
        archive = archive..'#zip:'
    elseif endswith(archive, '.tar.gz') or endswith(archive, '.tgz') then
        archive = archive..'#gzip:#tar:'
    elseif endswith(archive, '.gz') then
        archive = archive..'#gzip:'
    else
        -- This is just a directory, so add the directory separator
        archive = join(archive, '')
    end

    if not wx.wxFileSystem.HasHandlerForPath(archive) then
        return nil, 'Unknown archive type: '..archive
    end

    local fs = wx.wxFileSystem()
    fs.archive = archive

    function fs:ReadFile(filename)
        local f = self:OpenFile(filename)
        if not f then return false end
        local stream = f:GetStream()
        local t = {}
        repeat
            local buf = stream:Read(1024)
            local last_read = stream:LastRead()
            table.insert(t, buf:sub(0, last_read))
        until last_read < 1024
        f:delete()
        return table.concat(t)
    end

    return fs
end
