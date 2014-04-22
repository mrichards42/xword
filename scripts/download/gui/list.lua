local _R = mod_path(...)
local _RR = mod_path(..., 2)

local tablex = require 'pl.tablex'

local mgr = require(_RR .. 'manager')

local wxfb = require(_R .. 'wxFB')
local AutoWidthMixin = require 'wx.lib.mixins.listctrl'.AutoWidthMixin

local function DownloadList(parent)
    local self = wxfb.DownloadList(parent)    
    AutoWidthMixin(self.list)

    -- Columns
    self.list:InsertColumn(0, "#", wx.wxLIST_FORMAT_RIGHT, 30)
    self.list:InsertColumn(1, "Puzzle", wx.wxLIST_FORMAT_LEFT, 120)
    self.list:InsertColumn(2, "Date", wx.wxLIST_FORMAT_LEFT, 80)
    self.list:InsertColumn(3, "Status")

    -- Return a puzzle given a queue index
    -- Return complete puzzles, then the current puzzle, then queued puzzles
    local function get_puzzle(idx)
        -- Get the puzzle associated with this item
        return mgr.done[idx] or mgr.queue:at(idx - #mgr.done)
    end

    -- Return puzzle value by column
    local function get_col(puzzle, col)
        if puzzle then
            if col == 1 then
                return puzzle.name
            elseif col == 2 then
                return puzzle.date
            elseif col == 3 then
                return puzzle.status or "Queued"
            end
        end
        return '-'
    end

    -- Return the text for the list given a queue index and column
    local function get_text(idx, col)
        return get_col(get_puzzle(idx), col)
    end

    -- Override OnGetItemText, respecting sort order
    local sorted = {} -- Sorted puzzle index
    function self.list:OnGetItemText(item, col)
        item = item + 1
        local idx = sorted[item] and sorted[item] or item
        if col == 0 then -- Original order
            return tostring(idx)
        end
        return get_text(idx, col)
    end

    -- Sort
    --------------------------------------------------------------------------
    local sort_order = {
        {col=0, order=-1}, {col=1, order=-1},
        {col=2, order=-1}, {col=3, order=-1}
    }
    local function sort()
        -- Make sure the list has all indices.
        tablex.clear(sorted)
        for i=1,self.list:GetItemCount() do
            table.insert(sorted, i)
        end
        -- less = -1, greater = 1, same = 0
        local function cmp(a, b)
            return a < b and -1 or a > b and 1 or 0
        end
        table.sort(sorted, function(a, b)
            local puza = get_puzzle(a)
            local puzb = get_puzzle(b)
            for _, order in ipairs(sort_order) do
                local test
                if order.col == 0 then -- Original order
                    test = cmp(a, b)
                else
                    test = cmp(get_col(puza, order.col), get_col(puzb, order.col))
                end
                if test ~= 0 then
                    return test == order.order
                end
                -- Pass through to next column if equal
            end
            return false
        end)
        self.list:Refresh()
    end

    -- Events
    --------------------------------------------------------------------------
    function self:OnHeaderClick(evt)
        -- Add to sort order
        local col = evt:GetColumn()
        -- Remove this from the list and add to the front of the list
        for i, t in ipairs(sort_order) do
            if t.col == col then
                if i == 1 then -- Reverse sort if the current primary col
                    t.order = -t.order
                else -- Otherwise make primary, and sort ascending
                    t = table.remove(sort_order, i)
                    t.order = -1
                    table.insert(sort_order, 1, t)
                end
                sort()
                return
            end
        end
    end

    -- Update labels and list size
    local function update_count()
        --wx.wxGetApp():Yield(true)
        self.label:SetLabel(
            ("%d Queued, %d Error%s"):format(
                mgr.queue:length(),
                mgr.error_count,
                mgr.error_count == 1 and "" or "s")
        )
        self.list:SetItemCount(#mgr.done + mgr.queue:length())
        self.list:Refresh()
    end

    mgr:connect(mgr.EVT_DOWNLOAD_START, update_count)
    mgr:connect(mgr.EVT_DOWNLOAD_END, update_count)
    mgr:connect(mgr.EVT_CLEAR_HISTORY, update_count)
    --mgr:connect(mgr.EVT_PROGRESS, function() self.list:Refresh() end)
    mgr:connect(mgr.EVT_QUEUE_UPDATED, function() update_count() sort() end)

    function self:OnClearHistory()
        mgr:clear_history()
    end

    function self:OnCancelDownloads()
        mgr:clear()
    end

    update_count() -- Initial setup

    return self
end

return DownloadList
