local dir = require 'pl.dir'
local path = require 'pl.path'

local d = [[D:\C++\XWord\test_files]]

local function AddFile(filename)
    xword.frame:AddMenuItem({'Test Files'}, filename,
        function(evt)
            xword.frame:LoadPuzzle(path.join(d, filename))
        end
    )
end

local function init()
    for _, f in ipairs(dir.getfiles(d)) do
        AddFile(path.basename(f))
    end
end

init()
