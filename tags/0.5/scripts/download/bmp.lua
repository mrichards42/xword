download.onInit(function()

local path = require 'pl.path'

-- Lazy loading of images
local mt = { __imgdir = path.join(xword.scriptsdir, 'download', 'images') }

function mt:new(class, imgdir)
    local obj = { __class = class, __imgdir = imgdir }
    setmetatable(obj, mt)
    return obj
end

function mt:clear()
    for k,_ in pairs(self) do
        if not k:sub(1,2) == '__' then
            self[k] = nil -- deleting is handled by mt:__newindex
        end
    end
end

function mt:__index(name)
    local ret = rawget(getmetatable(self), name)
    if ret then return ret end
    local filename = path.join(self.__imgdir, name..'.png')
    if not path.isfile(filename) then
        -- This is a directory: create and return a new table with the same
        -- metatable as this table.
        local dirname = path.join(self.__imgdir, name)
        assert(path.isdir(dirname), 'File or directory does not exist: '..dirname)
        local t = mt:new(self.__class, path.join(self.__imgdir, name))
        rawset(self, name, t)
        return t
    end
    -- This is a file: create and return an image or a bitmap
    local img = self.__class(filename, wx.wxBITMAP_TYPE_PNG)
    rawset(self, name, img)
    return img
end

-- Delete the data in the image
function mt:__newindex(name, value)
    assert(value == nil)
    local img = rawget(self, name)
    if getmetatable(img) == mt then
        -- Recursively delete all image references in this table
        for k, _ in pairs(img) do
            if not k:sub(1,2) == '__' then
                img[k] = nil
            end
        end
    elseif img then
        img:delete()
    end
    rawset(self, name, value)
end


-- The images in the images folder (wxBitmap and wxImage)
download.bmp = mt:new(wx.wxBitmap)
download.img = mt:new(wx.wxImage)

end) -- onInit

download.onClose(function()
    download.bmp:clear()
    download.bmp = nil
    download.img:clear()
    download.img = nil
end)
