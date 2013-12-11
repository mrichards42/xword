-- We don't wont to depend on lfs yet because it's a dll . . .
-- here's a simple join function
if wx then
    return function(...)
        local t = {}
        for _, s in ipairs({...}) do
            while s:sub(-1) == '/' or s:sub(-1) == '\\' do
                s = s:sub(0, -2)
            end
            table.insert(t, s)
        end
        return table.concat(t, string.char(wx.wxFileName.GetPathSeparator()))
    end
else
    return require 'pl.path'.join
end
