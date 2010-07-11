-- ===========================================================================
-- A simple wxListCtrl for displaying data
-- ===========================================================================

local ipairs, assert, type, tostring = ipairs, assert, type, tostring
local tinsert, tconcat = table.insert, table.concat
local band = bit.band
local wx = wx

module('wx.mixins')

--[[
-- columns is a list of column data:
{ {"Label1", "lookup1", wx.wxALIGN_WHATEVER, -1 }, ... }


-- To set the data, call list:SetData(data) with a table.
{ { name1 = "value_1a", name2 = "value_2a", ... },
  { name1 = "value_1b", name2 = "value_2b", ... }, ... }

The above data and labels would result in the following list:
Label1     Label2
--------   ------
value_1a   value_2a
value_1b   value_2b
]]

function DataListCtrl(list, columns, data)
    assert(band(list:GetWindowStyleFlag(), wx.wxLC_REPORT), "DataListCtrl can only be used in report mode")
    list.data = {}

    function list:SetColumnData(columns)
        self.cols = {}
        for _, col in ipairs(columns) do
            local c = {}
            c.label  = assert(col[1] or col.label)
            c.lookup = col[2] or col.lookup or c.label:lower()
            c.align  = col[3] or col.align or wx.wxLIST_FORMAT_LEFT
            c.width  = col[4] or col.width or -1
            c.minwidth = col[5] or col.minwidth or 80
            tinsert(self.cols, c)
        end
        self:SetData(self.data)
    end

    -- Row and col are 1-based
    function list:FindDataValue(row, col)
        local fieldname = self.cols[col].lookup
        local value = self.data[row][fieldname]
        if value == nil then
            return ""
        elseif type(value) == "table" then
            local result = {}
            for _, v in ipairs(value) do
                tinsert(result, tostring(v))
            end
            return tconcat(result, "; ")
        else
            return tostring(value)
        end
    end

    -- Populate the list ctrl with our data
    function list:SetData(data)
        self.data = assert(data)
        self:ClearAll()

        for i=1,#self.cols do
            self:InsertColumn(i-1, self.cols[i].label,
                                   self.cols[i].align,
                                   self.cols[i].width)
        end
        for i, pkg in ipairs(self.data) do
            local row = self:InsertItem(i-1, self:FindDataValue(i, 1))
            for col=2,#self.cols do
                self:SetItem(row, col-1, self:FindDataValue(i, col))
            end
        end

        -- Autosize the columns
        for i = 0, self:GetColumnCount()-1 do
            local col = self.cols[i+1]
            self:SetColumnWidth(i, col.width)
            if self:GetColumnWidth(i) < col.minwidth then
                self:SetColumnWidth(i, col.minwidth)
            end
        end
    end

    list:SetColumnData(columns)

    return list
end
