require "database.db"

database.index = {}
database.index.directories = {}

-- ----------------------------------------------------------------------------
-- Event handling
-- ----------------------------------------------------------------------------
--[[
local indexEvents = wx.wxEvtHandler()

-- On idle, scan through all puzzles in the puzzles directory and analyze their
-- contents.
local function onIdle(evt)
    local success, err = coroutine.resume(index.cachePuzzles)
    if not success then
        xword.Message(err)
        database.index.stop()
    end
end

local function onClose(evt)
    index.stop()
    xword.frame:Close()
end


database.index.start()
    index.isrunning = true
    xword.frame:PushEventHandler(indexEvents)
    indexEvents:Connect(wx.wxEVT_CLOSE_WINDOW, onClose)
    indexEvents:Connect(wx.wxEVT_IDLE, onIdle)
end


function database.index.stop()
    index.isrunning = false
    xword.frame:RemoveEventHandler(indexEvents)
    indexEvents:Disconnect(wx.wxEVT_CLOSE_WINDOW)
    indexEvents:Disconnect(wx.wxEVT_IDLE)
end
]]

-- ----------------------------------------------------------------------------
-- Puzzle index functions
-- ----------------------------------------------------------------------------

require 'date'
require 'lfs'
require 'database.db'



--[[
local function strptime(str, fmt)
    local d = wx.wxDateTime()
    d:ParseFormat(str, fmt)
    return date(d:FormatISODate())
end

-- Return the source display name and date given the filename
-- (without extension or path)
local function findSource(filename)
    for _, source in ipairs(download.sources) do
        if filename:sub(1, #source.prefix) == source.prefix then
            local d = strptime(filename:sub(#source.prefix + 1), download.puzformat)
            return source, d
        end
    end
    return
end
]]



