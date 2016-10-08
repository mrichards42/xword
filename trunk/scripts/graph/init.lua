local tablex = require "pl.tablex"
local path = require 'pl.path'
local makepath = require 'pl.dir'.makepath
local serialize = require "serialize"
local ShowDialog = require "graph.dialog"

-- The package table
graph = {}

require "graph.config"

-- ---------------------------------------------------------------------------
-- Load/Save data
-- ---------------------------------------------------------------------------

graph.filename = nil -- the filename

-- Return a filename from config/graph
local function get_filename()
    return path.join(
        graph.configdir,
        'data',
        path.basename(graph.filename) .. '.lua'
    )
end

function graph.save()
    if graph.filename then
        local filename = get_filename()
        -- Make the path
        local dirname = path.dirname(filename)
        if not path.exists(dirname) then
            makepath(dirname)
        end
        -- Save the file
        serialize.dump(graph.points, filename)
    end
end

function graph.load()
    local points = serialize.loadfile(get_filename())
    if points then
        graph.points = points
    end
end


-- ---------------------------------------------------------------------------
-- The graph data
-- ---------------------------------------------------------------------------

graph.points = {} -- { { time, correct, incorrect, blank, black }, [...] }

-- Clear data for a new puzzle
function graph.reset(is_new)
    graph.save()
    graph.filename = xword.frame.Filename
    tablex.clear(graph.points)
    if not xword.HasPuzzle(false) then return end
    -- Check to see if we already saved a copy
    if is_new then
        graph.load()
    end
end

-- Work around paused timer
function get_time()
    if xword.frame:IsTimerRunning() then
        return xword.frame.Time
    else
        local length = #graph.points
        -- Add the difference from the last timestamp
        if length > 0 then
            local last = graph.points[length]
            local difference = os.time() - last.timestamp
            -- Add to the previous time
            return last.time + difference
        else
            -- Likely 0 if the timer is stopped
            return xword.frame.Time
        end
    end
end

-- Return a table of data for the current time
function graph.make_point()
    local point = {
        time = get_time(),
        timestamp = os.time(),
        correct = 0,
        incorrect = 0,
        blank = 0,
        black = 0,
    }
    -- Check the grid
    square = xword.frame.Puzzle.Grid:First()
    while (square) do
        if square:IsMissing() then
        elseif square:IsBlack() then
            point.black = point.black + 1
        elseif square:IsBlank() then
            point.blank = point.blank + 1
        elseif square:Check() then
            point.correct = point.correct + 1
        else
            point.incorrect = point.incorrect + 1
        end
        square = square:Next()
    end
    return point
end

-- Add a point to our graph
function graph.add_point(evt)
    if evt then evt:Skip() end
    if not xword.HasPuzzle() then return end
    -- Is this the same puzzle?
    if graph.filename ~= xword.frame.Filename then
        graph.reset(true)
    end
    -- Add a point
    local old = graph.points[#graph.points]
    local new = graph.make_point()
    -- Did we just clear the grid?
    if new.correct == 0 and new.incorrect == 0 and xword.frame.Time == 0 then
        graph.reset()
        table.insert(graph.points, new)
    -- Was the puzzle already complete?
    elseif old and old.incorrect == 0 and old.blank == 0 then
        -- Do nothing
    -- Is this the same point as last time?
    elseif old and new and (
            old.incorrect == new.incorrect and
            old.correct == new.correct and
            old.black == new.black and
            old.blank == new.blank
        ) then
        -- Do nothing
    else
        table.insert(graph.points, new)
    end
end


-- ---------------------------------------------------------------------------
-- Init/Uninit
-- ---------------------------------------------------------------------------

function graph.init()
    -- Add to the menu
    xword.frame:AddMenuItem({'Tools'}, 'View Solving Graph',
        function(evt)
            -- Can't do anything unless we have a puzzle
            if not xword.HasPuzzle() then return end
            -- Make sure we have at least one point to plot
            graph.add_point()
            ShowDialog(graph.points)
        end
    )
    -- The puzzle letter event handler
    xword.frame:Connect(xword.wxEVT_PUZ_LETTER, graph.add_point)
    graph.load_config()
end

function graph.uninit()
    graph.save()
    -- Remove events
    xword.frame:RemoveMenuItem('Tools', 'View Solving Graph')
    xword.frame:Disconnect(xword.wxEVT_PUZ_LETTER)
    graph.save_config()
end

return graph
