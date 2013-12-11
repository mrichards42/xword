-- ============================================================================
-- py2lua
--
-- Transform python output for wxFormBuilder projects into lua code
-- Output files have a .lua extension and the same basename
-- This is very much a work in progress, and only has fixes to accommodate
-- current needs.
--
-- usage: pu2lua.lua [--test|-t] filenames ...
--        --test -t: Show a wxFrame that tests each converted component
--                   NB: This only really works for wxPanels
-- ============================================================================

local oldprint = print
require 'wx'
print = oldprint

-- Events we know how to handle
-- { wxPythonEventBinder = wxWidgetsEventId }
events = {
    EVT_CHOICE = 'EVT_COMMAND_CHOICE_SELECTED',
    EVT_CHECKBOX = 'EVT_COMMAND_CHECKBOX_CLICKED',
    EVT_BUTTON = 'EVT_COMMAND_BUTTON_CLICKED',
    EVT_SCROLL = { -- Multiple events must be Connected for wxEVT_SCROLL
        'EVT_SCROLL_TOP',
        'EVT_SCROLL_BOTTOM',
        'EVT_SCROLL_LINEUP',
        'EVT_SCROLL_LINEDOWN',
        'EVT_SCROLL_PAGEUP',
        'EVT_SCROLL_PAGEDOWN',
        'EVT_SCROLL_THUMBTRACK',
        'EVT_SCROLL_THUMBRELEASE',
        'EVT_SCROLL_CHANGED',
    },
    EVT_CLOSE = 'EVT_CLOSE_WINDOW'
}

-- Convert the wxFormBuilder python code to lua code
function convert(filename, writefile)
    local f = io.open(filename, 'r')
    local text = f:read('*a')
    f:close()

    -- Remove coding
    text = text:gsub('#.-coding:.-\n', '\n')


    -- Custom subclasses must require their class
    local needs_relative_require = false
    text = text:gsub('from (.-) import (.-)\n', function(file, class)
        if file:match("[\"']") or file:match('_R') then
            needs_relative_require = true
            if class:match("BmpButton") then
                 -- Insure that we have bmp if we have BmpButton
                return 'local bmp = require(_R .. "bmp")\n' ..
                       string.format('local %s = require(%s)\n', class, file)
            end
            return string.format('local %s = require(%s)\n', class, file)
        end
        return string.format('local %s = require %q\n', class, file)
    end)

    -- Remove other imports (wx libraries)
    text = text:gsub('import .-\n', '')


    -- Comments
    text = text:gsub('##', '--')
    text = text:gsub('%-%-#', '---')
    text = text:gsub('# ', '-- ')


    -- Strings
    text = text:gsub(' u"', ' "')
    text = text:gsub('wx%.EmptyString', '""')


    -- Events
    text = text:gsub('\t\t([%w_%.]+)%.Bind%( wx%.([%w_]+), self%.([%w_]+) %)',
    function(obj, evt, func)
        -- Look up in the event override table
        luaevents = events[evt]
        -- Try wx.wxEVT . . .
        if not luaevents then
            assert(wx['wx' .. evt], 'Unknown event: ' .. evt)
            luaevents = evt
        end
        if type(luaevents) ~= 'table' then
            luaevents = {luaevents}
        end
        local eventtext = {}
        for _, luaevent in ipairs(luaevents) do
            table.insert(eventtext, string.format('\t\t%s:Connect(wx.%s, function(evt) self:%s(evt) end)', obj, luaevent, func))
        end
        return table.concat(eventtext, '\n')
    end)


    -- Bitwise operators
    text = text:gsub('|', '+')


    -- Lists
    text = text:gsub('%[', '{')
    text = text:gsub('%]', '}')


    -- Add 'local' to local variables
    -- Only variables defined at function level (two indents) should be
    -- considered
    text = text:gsub('\t\t(%S+) =', function(obj)
        if obj:sub(1, 4) ~= 'self' then
            return '\t\tlocal ' .. obj .. ' ='
        end
    end)
    -- If there were any local variables before, we made them doubly local
    text = text:gsub('local local', 'local')



    -- Missing functions and constants
    text = text:gsub('wx.SL_INVERSE', '4096')
    text = text:gsub('AddSpacer%( %( (%d+), (%d+)%), (%d+)[^%)]+%)', 'Add(%1, %2, %3)')
    text = text:gsub('SetToolTipString', 'SetToolTip')
    text = text:gsub('wx.ICON%(([^%)]+)%)', '%1') -- the wxICON macro doesn't exist
    text = text:gsub('SetSizeHintsSz', 'SetSizeHints')
    


    -- Constructor
    text = text:gsub('[^\n]*%.__init__[^\n]*', function(constructor)
        constructor = constructor:gsub('([%w_%.]+)%.__init__ %( self,', 'local self = %1 (')
        constructor = constructor:gsub(', [%w_]+ = ', ', ')
        return constructor
    end)


    -- Turn class into a function
    local classes = {}
    text = text:gsub('class ([%w_]+)[^:]+:', function (class)
        table.insert(classes, class .. ' = ' .. class)
        return 'local function ' .. class .. '(parent)'
    end)
    text = text:gsub('def __init__[^%(]*%b():', '')
    -- Return forms as a table from this file
    text = text .. '\n\nreturn {\n    ' .. table.concat(classes, ',\n    ') .. '\n}'


    -- Destructor
    text = text:gsub('def __del__[^:]+:', '\t\treturn self\nend')
    text = text:gsub('\t\tpass', '')


    -- Dangling event-handling functions
    text = text:gsub('def [^:]+:', '')
    text = text:gsub('\t\event.Skip%(%)', '')
    text = text:gsub('-- Virtual event handlers, overide them in your derived class', '')


    -- Figure out function/table semantic differences
    -- It is a function if there are ()
    -- It is a member if not
    text = text:gsub('wx%.', 'wx_')
    text = text:gsub('%.([%w_]+)%(', ':%1(')
    text = text:gsub('wx_', 'wx%.wx')


    -- Indents to spaces
    text = text:gsub('\t\t', '    ')
    text = text:gsub('\t', '')


    -- Collapse multiple newlines
    text = text:gsub('\n\n+', '\n\n')


    -- Relative require
    if needs_relative_require then
        text = 'local _R = (string.match(..., "^.+%.") or ... .. ".") -- relative require' .. text
    end


    if writefile ~= false then
        filename2 = filename:gsub('%.py', '%.lua')
        f = io.open(filename2, 'w')
        f:write(text)
        f:close()
        return text, filename2
    end
    return text
end

-- Test converted files with a dummy frame
function test_frame(str)
    function dostring(str)
        return assert(loadstring(str))()
    end
    require 'wx'
    local classes = dostring(str)
    frame = wx.wxFrame(wx.NULL, wx.wxID_ANY, "Test")
    panel = wx.wxPanel(frame, wx.wxID_ANY)
    notebook = wx.wxNotebook(panel, wx.wxID_ANY)
    -- Add pages from each "class" to the notebook
    for name, func in pairs(classes) do
        local panel = func(notebook)
        notebook:AddPage(panel, name)
    end
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    sizer:Add(notebook, 1, wx.wxEXPAND + wx.wxALL, 10)
    panel:SetSizerAndFit(sizer)
    frame:Fit()
    frame:SetMinSize(frame:GetSize())
    frame:Show()
    wx.wxGetApp():MainLoop()
end

-- Convert files on the command line
function main()
    dotest = false
    all_files = {}
    for _, filename in ipairs(arg) do
        if filename == '--test' or filename == '-t' then
            dotest = true
        else
            local success, err = pcall(function()
                print('Converting file', filename)
                text, newfile = convert(filename)
                table.insert(all_files, text)
                print('Success! wxLua file is at: ', newfile)
            end)
            if not success then
                print('Failed!')
                print(err)
            end
        end
    end
    -- Run a test frame?
    if dotest then
        test_frame(table.concat(all_files, '\n'))
    end
end

main()
