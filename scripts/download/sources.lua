--- Puzzle Sources.
-- Returns a `SourceList` of puzzle sources from `config/download/sources.lua`
-- and `default_sources.lua`.
-- The SourceList can be customized using the `config` module.
-- @alias SourceList
-- @module sources

local _R = mod_path(...)

local puz = require 'luapuz'
local tablex = require 'pl.tablex'
local path = require 'pl.path'
local split = require 'pl.stringx'.split
local assert_arg = require 'pl.utils'.assert_arg
local serialize = require 'serialize'


local config = require(_R .. 'config')
local stats = require(_R .. 'stats')
local mgr = require(_R .. 'manager')

-- Remove characters that are not allowed in filenames
local function sanitize_name(text)
    local n = text:gsub('[?<>:*|"%%\t\r\n\v\f]', "")
    return n
end

-- Metatables
local Puzzle = {}     -- A single puzzle with a source and date
local Source = {}     -- A source with info on how to find it
local SourceList = {} -- The list of sources


--- A Single Puzzle.
-- Inherits from `Source`.
-- @type Puzzle

Puzzle.__index = function(self, key)
    return Puzzle[key] or self._source[key] -- Inherit from Puzzle then _source
end

-- This should only be called from `Source:get_puzzle`.
function Puzzle.new(source, date)
    local self = {
        date = date:copy(),
        url = source.url and source:get_url(date),
        filename = source.filename and source:get_filename(date),
        _source = source
    }
    setmetatable(self, Puzzle)
    return self
end

--- Open the puzzle in XWord.
-- If the puzzle does not exist download it first.
function Puzzle:open()
    -- Don't do anything if we're already downloading the puzzle
    if mgr.current and mgr.current.filename == self.filename then
        mgr.open_after_download = self.filename
        return
    end
    if self:exists() then
        SourceList.open(self.filename)
    else
        self:download{prepend=true, open=true}
    end
end

--- Does this puzzle exist on disk?
function Puzzle:exists()
    return stats.exists(self.filename)
end

--- Get a table of data for the download task.
-- @return A copy of this puzzle suitable for passing to `Manager:add`.
function Puzzle:get_download_data()
    local obj = tablex.deepcopy(self._source)
    obj.date = self.date:format("%m/%d/%Y") -- task.post won't keep metatables
    obj.url = self.url
    obj.filename = self.filename
    return obj
end

--- Download the puzzle.
-- @param opts Additional options for `Manager:add`.
function Puzzle:download(opts)
    opts = opts or {}
    opts[1] = {self}
    mgr:add(opts)
end


--- A Puzzle Source
-- @type Source
Source.__index = Source

--- A descriptive name.
-- @field Source.name

--- A unique id.  Automatically generated from `Source.name` if blank.
-- @field Source.id

--- The url.  Uses `date.fmt` codes.
-- @field Source.url

--- A table of days to expect puzzles. days[1] = Monday.
-- @field Source.days

--- The subdirectory to save puzzles.  Defaults to `Source.name`.
-- @field Source.directoryname

--- The local filename of a puzzle. to save puzzles.  Uses `date.fmt` codes.
-- The full path of a puzzle can be found by joining
-- `config.puzzle_directory`, `Source.directoryname`, and date:format(Source.filename)
-- @field Source.filename

--- An optional custom download function.
-- @field Source.func

--- A table of optional user-supplied data.
-- Used in conjunction with `Source.func`.
-- @field Source.fields

--- Is this source disabled?
-- @field Source.disabled

-- This should only be called from `SourceList:insert`.
function Source.new(obj)
    setmetatable(obj, Source)
    return obj
end

--- Get the url of a puzzle on this date.
-- @param date The `date`.
-- @return A url.
function Source:get_url(date)
    if type(self.url) == 'string' then
        return date:format(self.url)
    else
        return self.url
    end
end

--- Get the date format for the local filename of a puzzle.
-- @return A date format string.
function Source:get_filename_fmt()
    return path.join(
        config.puzzle_directory,
        config.separate_directories and sanitize_name(self.directoryname or self.name) or '',
        self.filename
    )
end


--- Get the local filename of a puzzle on this date.
-- @param date The `date`.
-- @return The filename.
function Source:get_filename(date)
    return date:format(self:get_filename_fmt())
end

--- Get a puzzle for this date.
-- @param date The `date`.
-- @return A `Puzzle`.
function Source:get_puzzle(date)
    return Puzzle.new(self, date)
end

--- Get a table of puzzles between two dates.
-- @param start The first `date`.
-- @param[opt=start] end_ The last `date` (inclusive).
-- @param[opt=false] only_missing Only return missing puzzles.
-- @param[opt={}] t (Internal use) A table to use for results.
-- @return A table of `Puzzle` objects.
function Source:get_puzzles(start, end_, only_missing, t)
    end_ = end_ or start
    local puzzles = t or {}
    local d = start:copy()
    while d <= end_ do
        if self:has_puzzle(d) then
            local puzzle = self:get_puzzle(d)
            if not (only_missing and puzzle:exists()) then
                table.insert(puzzles, puzzle)
            end
        end
        d:adddays(1)
    end
    return puzzles
end

--- Download puzzles between two dates.
-- See below for usage and an option table overload.
-- @param start The first `date`.
-- @param[opt=start] end_ The last `date` (inclusive).
-- @function Source:download

--- Download puzzles between two dates.
-- Does not redownload existing puzzles.
-- @param opts A table of options.
-- @see SourceList:download
-- @function Source:download
function Source:download(opts, ...)
    -- Truly the same as SourceList.download
    SourceList.download(self, opts, ...)
end

--- Is there a puzzle available for the date(s)?
-- @param d1 First `date`.
-- @param[opt] d2 Second `date`.  Must be on or after d1.
-- @return true/false
function Source:has_puzzle(d1, d2)
    if not d2 or d1 == d2 then -- Single date
        return self.days[d1:getisoweekday()]
    else -- Multiple dates
        local diff = math.ceil((d2 - d1):spandays())
        if diff >= 6 then -- A week or more; should always return true
            return tablex.find(self.days, true) and true or false
        else -- Less thank a week; check each day
            local day = d1:getisoweekday()
            while true do
                if self.days[day] then
                    return true
                elseif day == d2:getisoweekday() then
                    return false
                end
                day = (day % 7) + 1 -- Wrap from 7 (sunday) to 1 (monday)
            end
        end
    end
end

--- How many puzzles *exist* between these dates?
-- @param d1 First `date`.
-- @param[opt] d2 Second `date`.  Must be on or after d1.
-- @return existing puzzles, missing puzzles
function Source:puzzle_count(d1, d2)
    if not d2 or d1 == d2 then -- Single date
        if stats.exists(self:get_filename(d1)) then
            return 1,0
        else
            return 0,1
        end
    else -- Multiple dates
        local count = 0
        local total = 0
        local d = d1:copy()
        -- NB: get_filename itself is fairly expensive, since get_filename_fmt
        -- is called each time, so just call it once here
        local fmt = self:get_filename_fmt()
        while d <= d2 do
            if self:has_puzzle(d) then
                total = total + 1
                if stats.exists(d:format(fmt)) then
                    count = count + 1
                end
            end
            d:adddays(1)
        end
        return count, total - count
    end
end

--- The List of Sources
-- @type SourceList

SourceList.__index = SourceList

function SourceList.new(sources)
    -- Set the metatable
    local obj = {}
    obj._order = {}
    setmetatable(obj, SourceList)
    -- Insert sources in order
    for _, p in ipairs(sources) do
        obj:insert(p)
    end
    return obj
end

--- Iterate enabled sources
-- @return an iterator yielding: id, source
function SourceList:iter()
    local i = 0
    local n = #self._order
    return function ()
        while i <= n do
            i = i + 1
            if i <= n then
                local key = self._order[i]
                if not self[key].disabled then
                    return key, self[key]
                end
            end
        end
    end
end

--- Iterate all sources
-- @return an iterator yielding: id, source
function SourceList:iterall()
    local i = 0
    local n = #self._order
    return function ()
        i = i + 1
        if i <= n then
            local key = self._order[i]
            return key, self[key]
        end
    end
end

--- Get a source by id or index
-- @param key The index of the puzzle, or the internal puzzle id.
-- @return The `Source`.
function SourceList:get(key)
    local p = self[key]
    if p then return p end
    local id = self._order[key]
    if key then return self[id] end
end

--- Insert a source and give it the Source metatable.
-- @param src A `Source` table.
-- @param[opt=last] idx Insert before this index.
function SourceList:insert(src, idx)
    src = Source.new(src)
    -- Generate an id
    src.id = src.id or src.name
    local i = 2
    while self[src.id] ~= nil do
        src.id = src.name .. tostring(i)
        i = i + 1
    end
    -- Insert
    self[src.id] = src
    table.insert(self._order, idx or #self._order + 1, src.id)
end

--- Remove a source.
-- @param obj An id or a `Source` object.
function SourceList:remove(obj)
    local id = type(obj) == 'table' and obj.id or obj
    for i, srcid in ipairs(self._order) do
        if srcid == id then
            table.remove(self._order, i)
        end
    end
    self[id] = nil
end

--- Get a table of puzzles between two dates.
-- @param start The first `date`.
-- @param[opt=start] end_ The last `date` (inclusive).
-- @param[opt=false] only_missing Only return missing puzzles.
-- @return A table of `Puzzle` objects from all enabled sources.
function SourceList:get_puzzles(start, end_, only_missing)
    local puzzles = {}
    for _, source in self:iter() do
        source:get_puzzles(start, end_, only_missing, puzzles)
    end
    return puzzles
end

--- Download puzzles between two dates.
-- See below for usage and an option table overload.
-- @param start The first `date`.
-- @param[opt=start] end_ The last `date` (inclusive).
-- @function SourceList:download

--- Download puzzles between two dates.
-- Does not redownload existing puzzles.
-- @param opts A table of options.
-- @param opts.1 The first `date`.
-- @param[opt=start] opts.2 The last `date` (inclusive).
-- @param opts.prepend Download these puzzles before others in the queue.
-- @param opts.open Open the first puzzle in the list when complete.
-- @usage
-- -- Download all puzzles from enabled sources for October.
-- source_list:download(date('10/1/2013'), date('10/31/2013'))
-- -- Download all puzzles from enabled sources for November
-- -- and add to the front of the queue.
-- source_list:download{date('11/1/2013'), date('11/30/2013'), prepend=true}
function SourceList:download(opts, ...)
    local start, end_ = opts, ...
    if opts and #opts > 0 then
        start = opts[1] or opts.start
        end_ = opts[2] or opts.end_
        -- Clear the array part of opts; leave the hash part for manager:add
        for i=#opts,1,-1 do opts[i] = nil end
    else
        opts = {}
    end
    -- Check the argument
    local tp = type(start)
    if tp ~= 'table' or not start.daynum then
        error(("argument 1 expected a date, got a '%s'"):format(tp), 2)
    end
    -- Get missing puzzles and download
    opts[1] = self:get_puzzles(start, end_, true) -- Only missing puzzles
    mgr:add(opts)
end

-------------------------------------------------------------------------------
-- Static/module-level functions
-- @section static

--- Get the default sources.
-- Reloads the default sources, creating a copy of the `SourceList` returned by
-- this module.
-- @return A `SourceList`.
function SourceList.get_default_sources()
    -- Try to load the sources from the config directory
    local sources = serialize.loadfile(path.join(xword.configdir, 'download', 'sources.lua'))
    -- Fallback on the default sources
    if not sources then
        sources = dofile(path.join(xword.scriptsdir, unpack(split(_R, '%.')), 'default_sources.lua'))
    end
    return SourceList.new(sources)
end

--- Open a puzzle in XWord.
-- If not a valid puzzle, try to open using the default program for this type.
-- @param filename The puzzle.
function SourceList.open(filename)
    if not path.isfile(filename) then return end
    if puz.Puzzle.CanLoad(filename) then
        -- Open the puzzle
        xword.frame:LoadPuzzle(filename)
    else
        -- If we can't open, try to open with the default program
        local ext = filename:match('%.([^%.]+)')
        local ft = wx.wxTheMimeTypesManager:GetFileTypeFromExtension(ext)
        if ft then
            wx.wxExecute(ft:GetOpenCommand(filename), wx.wxEXEC_ASYNC)
        end
    end
end

-- Return puzzle sources
return SourceList.get_default_sources()
