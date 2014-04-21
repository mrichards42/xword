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
local path = require 'pl.path'

-- Events we know how to handle
-- { wxPythonEventBinder = wxWidgetsEventId }
local events = {
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

-- Missing constants, and other replacements
local replace = {
    -- Bitwise operators
    {'|', '+'},
    -- Booleans
    {'True', 'true'},
    {'False', 'false'},
    -- Lists
    {'%[', '{'},
    {'%]', '}'},
    -- Missing functions
    {'AddSpacer%( %( (%d+), (%d+)%), (%d+)[^%)]+%)', 'Add(%1, %2, %3)'},
    {'SetToolTipString', 'SetToolTip'},
    {'wx.ICON%(([^%)]+)%)', '%1'}, -- the wxICON macro doesn't exist
    {'SetSizeHintsSz', 'SetSizeHints'},
    -- Constants
    {'wx.SL_INVERSE', 4096},
    {'wx.ST_ELLIPSIZE_START', 4},
    {'wx.ST_ELLIPSIZE_MIDDLE', 8},
    {'wx.ST_ELLIPSIZE_END',   16},
}

-- Convert the wxFormBuilder python code to lua code
local function convert(filename, writefile, luafile)
    local f = assert(io.open(filename, 'r'))
    local text = f:read('*a')
    f:close()

    -- Remove coding
    text = text:gsub('#.-coding:.-\n', '\n')


    -- Custom subclasses must require their class
    local needs_relative_require = false
    text = text:gsub('from (.-) import (.-)\n', function(file, class)
        -- If we use _R we need a relative require statement
        if file:match('_R') then
            needs_relative_require = true
        end
        -- Add quotes if necessary
        if not file:match("[\"']") then
            file = string.format("%q", file)
        end
        return string.format('local %s = require(%s)\n', class, file)
    end)
    -- Relative require
    if needs_relative_require then
        text = 'local _R = (...):match("^.+%.") or "" -- relative require\n' .. text
    end

    -- Replace the wx.ICON macro with require_bmp from wx.lib.bmp
    local needs_bmp = false
    text = text:gsub("wx%.ICON(%b())", function(name)
        needs_bmp = true
        name = name:match("^%(%s*(.-)%s*%)$") -- Remove parentheses
        -- Add quotes if necessary
        if not name:match("[\"']") then
            name = string.format("%q", name)
        end
        return string.format("require_bmp(%s)", name)
    end)
    if needs_bmp then
        text = 'local require_bmp = require("wx.lib.bmp") -- require function for images\n' .. text
    end

    -- Add empty string for button label if using a stock id
    text = text:gsub('(wx.Button%( self, wx.ID_%S+) %)', '%1, "" )')

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
        -- Check for the splitter OnIdle function
        if evt == 'EVT_IDLE' and func:match("OnIdle") then
            return ''
        end
        -- Look up in the event override table
        local luaevents = events[evt]
        -- Try wx.wxEVT . . .
        if not luaevents then
            local cmd_evt = evt:gsub("EVT_", "EVT_COMMAND_")
            if wx['wx' .. cmd_evt] then
                luaevents = cmd_evt
            else
                assert(wx['wx' .. evt], 'Unknown event: ' .. evt)
                luaevents = evt
            end
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

    -- Other replacements
    for _, t in ipairs(replace) do
        text = text:gsub(t[1], tostring(t[2]))
    end

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
        return 'function wxfb.' .. class .. '(parent)'
    end)
    text = text:gsub('def __init__[^%(]*%b():', '')
    -- Return forms as a table from this file
    text = 'local wxfb = {} -- Table to hold classes\n\n' .. text .. '\n\nreturn wxfb'

    -- Destructor
    text = text:gsub('\tdef __del__.-:.-\t\n', '\t\treturn self\nend')

    -- Dangling event-handling functions
    text = text:gsub('\tdef [^:]+:.-\t\n', '')
    text = text:gsub('-- Virtual event handlers, overide them in your derived class', '')


    -- Replace wx.XXX with wx.wxXXXX
    -- Figure out function/table semantic differences:
    --   if (), it's a function, otherwise table
    text = text:gsub('wx%.', 'wx_')
    text = text:gsub('%.([%w_]+)%(', ':%1(')
    text = text:gsub('wx_', 'wx.wx')
    text = text:gsub(' wxfb:', ' wxfb.')
    text = text:gsub('require%("wx%.wx', 'require("wx.')


    -- Indents to spaces
    text = text:gsub('\t\t', '    ')
    text = text:gsub('\t', '')


    -- Collapse multiple newlines
    text = text:gsub('\n\n+', '\n\n')

    if writefile ~= false then
        if not luafile then
            luafile = filename:gsub('%.py', '%.lua')
        end
        f = io.open(luafile, 'w')
        -- Keep track of filename and modification time
        f:write(([[
-- Converted from python by py2lua
-- python file: %s
-- modtime: %s
-- ----------------------------------

]]):format(path.basename(filename), path.getmtime(filename)))
        -- Write converted text
        f:write(text)
        f:close()
        return text, luafile
    end
    return text
end

-- Test converted files with a dummy frame
-- @param files A table mapping filenames to file text
local function test_frame(files)
    -- Load the files and save the classes
    local classes = {}
    for filename, str in pairs(files) do
        -- Don't worry about errors if we have loading errors
        local r = require
        function require(...)
            local success, result = pcall(r, ...)
            if success then
                return result
            else
                -- Return an object that returns itself whe indexed or called
                local nil_obj = {}
                nil_obj.__call = function() return nil_obj end
                nil_obj.__index = function() return nil_obj end
                setmetatable(nil_obj, nil_obj)
                return nil_obj
            end
        end
        filename = filename:gsub('%..-$', ''):gsub('[/\\]', '.')
        for k, v in pairs(assert(loadstring(str))(filename)) do
            classes[k] = v
        end
    end
    -- Make a notebook for the classes
    local frame = wx.wxFrame(wx.NULL, wx.wxID_ANY, "Test")
    local panel = wx.wxPanel(frame, wx.wxID_ANY)
    local notebook = wx.wxNotebook(panel, wx.wxID_ANY)
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

-- ----------------------------------------------------------------------------
-- Convert files on the command line
-- ----------------------------------------------------------------------------
local function main()
    local dotest = false
    local is_debug = true
    local all_files = {}
    for _, opt in ipairs(arg) do
        if opt == '--test' or filename == '-t' then
            dotest = true
        else
            local filename = opt
            local success, err = pcall(function()
                print('Converting file', filename)
                local text, newfile = convert(filename)
                all_files[filename] = text
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
        test_frame(all_files)
    end
end

-- ----------------------------------------------------------------------------
-- Auto Update
-- ----------------------------------------------------------------------------

-- Determine if the python file (source of the lua file) was changed or is
-- missing.
-- Returns true, python file if modified or false if not modified
local function is_modified(luafile)
    local f = io.open(luafile, 'r')
    if f then
        f:read("*line") -- py2lua line
        local pyfile = f:read("*line"):match("--.*:%s+(.*)") -- python file
        local mtime = tonumber(f:read("*line"):match("--.*:%s+(.*)")) -- modified time
        f:close()
        if pyfile and mtime then
            -- Get the full path of the python file
            pyfile = path.join(path.dirname(luafile), pyfile)
            -- Compare modification times
            if path.getmtime(pyfile) > mtime then
                return true, pyfile
            else
                return false
            end
        end
    end
    -- If we haven't returned, assume we couldn't find a lua file, and thus
    -- need to generate it.
    return true, path.splitext(luafile) .. ".py"
end

local function update(modname)
    -- Look for the lua file
    local luafile = package.searchpath(modname, package.path)
    -- If there is no luafile, try for a python file with the same search path
    -- Convert the extension back to lua
    if not luafile then
        local pypath = package.path:gsub('%.lua', '%.py')
        luafile = package.searchpath(modname, pypath):gsub('.py', '.lua')
    end
    -- Has this file been modified (or does it not exist)?
    local modified, pyfile = is_modified(luafile)
    if modified then
        local success, err = pcall(function()
            print('Updating wxFB from python', pyfile)
            convert(pyfile, true, luafile)
            print('Success! wxLua file is at: ', luafile)
        end)
        if not success then
            print('Failed!')
            print(err)
        end
    end
end

-- ----------------------------------------------------------------------------
-- Main
-- ----------------------------------------------------------------------------
if arg then
    -- py2lua filename
    main()
else
    -- Used as a module
    -- Hook require to auto-update scripts
    local old_require = require
    function require(modname)
        if package.loaded[modname] then
            return package.loaded[modname]
        end
        -- Check for a wxFB python file
        local pypath = package.path:gsub('%.lua', '%.py')
        local pyfile = package.searchpath(modname, pypath)
        if pyfile then
            local f = io.open(pyfile, 'r')
            if f then
                -- The 4th line should have this magic:
                f:read('*line')
                f:read('*line')
                f:read('*line')
                local is_fb = f:read('*line'):match("wxFormBuilder")
                f:close()
                if is_fb then
                    update(modname)
                end
            end
        end
        return old_require(modname)
    end
end