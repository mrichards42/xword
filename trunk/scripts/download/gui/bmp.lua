--- Lazy image loading
-- @classmod Bitmap
-- @usage
-- local bmp = require 'bmp'
-- -- Get the "arrow.png" image:
-- bmp.arrow
-- -- Get a sub bitmap of the "arrow.png" image:
-- bmp.sub.arrow(0, 0, 16, 16)
-- -- Get the "arrows/left.png" image:
-- bmp.arrows.left

local path = require 'pl.path'

local M = {}

function M.new(class, imgdir)
    return setmetatable({ _class = class, _imgdir = imgdir }, M)
end

function M.__index(self, name)
    local filename = path.join(self._imgdir, name..'.png')
    if path.isfile(filename) then
        -- Create and return the object
        return self._class(filename, wx.wxBITMAP_TYPE_PNG)
    end
    -- Is this a directory?
    local dirname = path.join(self.__imgdir, name)
    if not path.isdir(dirname) then
        error('File or directory does not exist: '..dirname, 2)
    end
    -- Create and return a new table to represent this directory
    local t = self.new(self._class, path.join(self._imgdir, name))
    self[name] = t
    return t
end

-- A loader for images in the images folder
local loader = M.new(wx.wxBitmap, path.join(xword.scriptsdir, 'download', 'images'))

--- Return a function that loads a bmp and returns a sub-bitmap
local function do_sub(filename, flag)
    return function(...)
        local bmp = loader._class(filename, flag)
        local ret = bmp:GetSubBitmap(wx.wxRect(...))
        bmp:delete()
        return ret
    end
end
loader.sub = M.new(do_sub, loader._imgdir)

return loader
