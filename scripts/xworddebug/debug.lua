-- ============================================================================
-- debug.lua
--     Provides the xword.debug function for lua debug output
-- ============================================================================
local tablex = require 'pl.tablex'

local sep = '    '
local function get_message(...)
    local args = { n = select("#", ...), ... }
    if type(args[1]) == 'string' then
        -- Get the number of format params are in the first argument
        local _, field_count = args[1]:gsub('%%%%', ''):gsub('%%', '')
        -- Get the difference between this and the number of arguments
        local extra_args = args.n - 1 - field_count
        -- Format the string
        local success, msg = pcall(string.format, ...)
        if success then
            -- Add arguments that did not get incorporated
            if extra_args > 0 then
                local t = {msg}
                for i=field_count+2,args.n do
                    table.insert(t, tostring(args[i]))
                end
                return table.concat(t, sep)
            else
                return msg
            end
        end
    end
    return table.concat(tablex.imap(tostring, args), sep)
end

-- The dialog
local dlg
local function get_dialog()
    if not dlg then
        dlg = wx.wxFrame(xword.frame, wx.wxID_ANY, 'Lua Debug',
            wx.wxDefaultPosition, wx.wxDefaultSize,
            wx.wxFRAME_TOOL_WINDOW + wx.wxFRAME_FLOAT_ON_PARENT +
            wx.wxDEFAULT_FRAME_STYLE
        )
        local panel = wx.wxPanel(dlg, wx.wxID_ANY)
        local sizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
        local text = wx.wxTextCtrl(panel, wx.wxID_ANY, "", wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxTE_MULTILINE)
        sizer:Add(text, 1, wx.wxEXPAND + wx.wxALL, 10)
        panel:SetSizer(sizer)
        function dlg:add_message(msg)
            text:AppendText(msg .. '\n')
        end
        local dlgsizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
        dlgsizer:Add(panel, 1, wx.wxEXPAND)
        dlg:SetSizer(dlgsizer)
    end
    return dlg
end

--[[
    xword.debug (the public function)
    Use in place of print to see output

    Call as in string.format:
        xword.debug('%s: %d', 'hello world', 15)
            -> "hello world: 15"

    Extra arguments will be added to the string:
        xword.debug('%s: %d', 'hello world', 15, "interesting")
            -> "hello world: 15    interesting"
]]
function xword.debug(...)
    local dlg = get_dialog()
    dlg:add_message(get_message(...))
    dlg:Show()
end
