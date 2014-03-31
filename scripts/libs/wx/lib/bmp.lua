--- A require function for wxBitmaps.

local path = require 'pl.path'

--- Return the full path of a file with this name and extension
local function search(name, ext)
    -- Make into a filename
    local filename = name:gsub('%.',path.sep)
    -- Search package.path for this file's extension
    return package.searchpath(filename,package.path:gsub("%.lua", ext))
end

--- Return the full path of a file with the supplied name
local function find_file(name)
    -- Get the extension
    local ext
    name, ext = path.splitext(name)
    -- Search for a file with the given extension
    local res
    if #ext > 1 then
        res = search(name, ext)
        if res then return res end
    end
    -- Then search for a png.
    return search(name .. ext, ".png")
end

--- Load and return a png as a bitmap.
-- @param name A module name or filename or an image.
--   If name ends with a file extension, try to load the file.
--   Otherwise assume .png
-- @param[opt=wxBitmap] class The class to use.  Can be wxBitmap or wxImage
-- @return a wxBitmap or wxImage
-- @usage
-- -- Load images/my_bitmap.png
-- my_bitmap = require("wx.lib.bmp")("images.my_bitmap")
-- -- Load images/my_bitmap.bmp
-- my_bitmap = require("wx.lib.bmp")("images.my_bitmap.bmp")
-- @function require_bmp
return function(name, class)
    class = class or wx.wxBitmap
    local filename = find_file(name)
    if filename then
        return class(filename)
    end
    wx.wxLogWarning("Unable to load bitmap: " .. tostring(name))
    return class()
end