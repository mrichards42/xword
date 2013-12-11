-- ============================================================================
-- interp.lua
--     A wxLua script that functions as a very primitive shell.
-- ============================================================================


local tablex = require 'pl.tablex'
local stringx = require 'pl.stringx'

-- locals
local fixedWidthFont = wx.wxFont(8, wx.wxFONTFAMILY_MODERN,
                                 wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_NORMAL)


-- tostring conversion functions for wxwidgets types
local _tostring = tostring
local conv = {}
local tostring = function(obj)
    local t = wxlua.type(obj)
    if t == "string" then
        return obj
    elseif conv[t] then
        return conv[t](obj) .. '    '.._tostring(obj)
    else
        return _tostring(obj)
    end
end

function conv.wxSize(obj)
    return string.format("wxSize(%d, %d)", obj.Width, obj.Height)
end
function conv.wxPoint(obj)
    return string.format("wxPoint(%d, %d)", obj.X, obj.Y)
end
function conv.wxRect(obj)
    return string.format("wxRect(%d, %d, %d, %d)", obj.X, obj.Y, obj.Width, obj.Height)
end
function conv.wxAuiPaneInfo(obj)
    return string.format([[wxAuiPaneInfo(
    -- General
    name       = %s
    caption    = %s
    window     = %s
    IsOk()    => %s
    IsShown() => %s
    HasCaption() => %s

    -- Size / Position
    rect           = %s
    best_size      = %s
    min_size       = %s
    max_size       = %s
    IsFixed()     => %s
    IsResizable() => %s

    -- Dock
    IsDocked() => %s
        IsDockable()   => %s
        dock_direction  = %d 
        dock_layer      = %d
        dock_row        = %d
        dock_pos        = %d
        dock_proportion = %d

    -- Floating
    IsFloating() => %s
        IsFloatable()  => %s
        floating_pos    = %s
        floating_size   = %s
        frame           = %s
    ]], obj.name,
        obj.caption,
        tostring(obj.window),
        tostring(obj:IsOk()),
        tostring(obj:IsShown()),
        tostring(obj:HasCaption()),

        tostring(obj.rect),
        tostring(obj.best_size),
        tostring(obj.min_size),
        tostring(obj.max_size),
        tostring(obj:IsFixed()),
        tostring(obj:IsResizable()),

        tostring(obj:IsDocked()),
        tostring(obj:IsTopDockable() or
                 obj:IsBottomDockable() or
                 obj:IsLeftDockable() or
                 obj:IsRightDockable()),
        obj.dock_direction,
        obj.dock_layer,
        obj.dock_row,
        obj.dock_pos,
        obj.dock_proportion,

        tostring(obj:IsFloating()),
        tostring(obj:IsFloatable()),
        tostring(obj.floating_pos),
        tostring(obj.floating_size),
        tostring(obj.frame)
    )
end


-- user-defined methods
local function GetUserMethodsRegistry()
    -- Find the registry table that is user-defined wxLua functions

    -- Get the address of "MyFrame"
    local name, addr, num = tostring(xword.frame):match("%[(.+)%((%x+), (%d+)%)%]")
    -- Search the registry for a table that looks like this:
    --[[
        userdata = {
            userdata (MyFrame): { method_name = function, [...] },
            userdata: { method_name = function, [...] },
            [...]
        }
    ]]
    -- We know that MyFrame should be in this table
    -- There are other similar tables

    for k, t in pairs(debug.getregistry()) do
        if type(k) == 'userdata' and type(t) == 'table' then
            for cls, funcs in pairs(t) do
                if type(cls) ~= 'userdata' or type(funcs) ~= 'table' then
                    break
                end
                -- Is this MyFrame?
                if tostring(cls):match('userdata: (%x+)') == addr and 
                    funcs.AddMenuItem then
                    return t
                end
            end
        end
    end
end

local function GetUserDefinedMethods(obj)
    print(obj)
    -- Make sure we know where the registry table is
    if not xword_wxlua_user_methods then
        xword_wxlua_user_methods = GetUserMethodsRegistry()
    end
    local name, addr, num = tostring(obj):match("%[(.+)%((%x+), (%d+)%)%]")
    for cls, funcs in pairs(xword_wxlua_user_methods) do
        print(cls, obj)
        if tostring(cls):match('userdata: (%x+)') == addr then
            f = {}
            for k, _ in pairs(funcs) do
                f[k] = obj[k]
            end
            return f
        end
    end
end

-- The Interpreter
local function createInterpreter(parent, dlg, id, value, pos, size, style)
    local id = id or wx.wxID_ANY
    local value = value or "Welcome to lua!\n"
    local pos = pos or wx.wxDefaultPosition
    local size = size or wx.wxDefaultSize
    local style = (style or 0) + wx.wxTE_PROCESS_ENTER
                               + wx.wxTE_PROCESS_TAB
                               + wx.wxTE_MULTILINE
                               + wx.wxTE_DONTWRAP
    local interpreter = wx.wxTextCtrl(
        parent, wx.wxID_ANY, value, pos, size, style
    )
    interpreter:SetFont(fixedWidthFont)

    local history = {''}
    local historyNum = #history + 1

    local prompt = '> '
    function interpreter.showPrompt()
        interpreter:AppendText(prompt)
    end
    local showPrompt = interpreter.showPrompt

    local getLine = function()
        return interpreter:GetLineText(
            interpreter:GetNumberOfLines() - 1
        ):sub(#prompt)
    end

    showPrompt()

    dlg.STOP_PRINT = false
    interpreter:Connect(wx.wxID_ANY, wx.wxEVT_KEY_DOWN,
        function(evt)
            local char = evt:GetKeyCode()
            dlg.STOP_PRINT = false
            evt:Skip(false)
            if char == wx.WXK_TAB then
                interpreter:WriteText('    ')
            -- Enter a command
            elseif char == wx.WXK_RETURN or char == wx.WXK_NUMPAD_ENTER then
                local text = getLine()
                table.insert(history, text)
                historyNum = #history + 1
                dlg.execString(text)
                showPrompt()
            -- Line history
            elseif char == wx.WXK_DOWN or char == wx.WXK_UP then
                if char == wx.WXK_UP then
                    historyNum = historyNum - 1
                    if historyNum < 1 then historyNum = 1 end
                else
                    historyNum = historyNum + 1
                    if historyNum > #history then historyNum = #history end
                end
                local historyText = history[historyNum]
                if historyText then
                    local lastChar = interpreter:GetLastPosition()
                    interpreter:Replace(lastChar - #getLine(),
                                        lastChar,
                                        historyText)
                end
            -- Ctrl+C to stop printing
            elseif char == string.byte('C') and evt:GetModifiers() == wx.wxMOD_CONTROL then
                dlg.STOP_PRINT = true
                interpreter:Copy(); -- Copy text to the keyboard
            else
                evt:Skip()
            end
        end)

    return interpreter
end


local function createTextCtrl(parent, dlg, id, value, pos, size, style)
    local id = id or wx.wxID_ANY
    local value = value or
                "-- Enter a function or other multi-line chunk here\n" ..
                "-- then press 'Run Script'\n"
    local pos = pos or wx.wxDefaultPosition
    local size = size or wx.wxDefaultSize
    local style = (style or 0) + wx.wxTE_PROCESS_ENTER
                               + wx.wxTE_PROCESS_TAB
                               + wx.wxTE_MULTILINE
                               + wx.wxTE_DONTWRAP
    local ctrl = wx.wxTextCtrl(
        parent, wx.wxID_ANY, value, pos, size, style
    )
    ctrl:SetFont(fixedWidthFont)

    function ctrl.getLineStart()
        local c = ctrl:GetInsertionPoint() - 1
        while c > -1 and ctrl:GetRange(c, c+1) ~= '\n' do
            c = c - 1
        end
        return c + 1
    end

    function ctrl.getLineEnd()
        local c = ctrl:GetInsertionPoint()
        while c < ctrl:GetLastPosition() and ctrl:GetRange(c, c+1) ~= '\n' do
            c = c + 1
        end
        return c
    end

    function ctrl.getCurrentLine()
        return ctrl:GetRange(ctrl.getLineStart(), ctrl.getLineEnd())
    end

    function ctrl.getIndent()
        local spaces = ctrl.getCurrentLine():match('^( *)')
        return math.floor(#spaces / 4)
    end

    ctrl:Connect(wx.wxID_ANY, wx.wxEVT_KEY_DOWN,
        function(evt)
            local char = evt:GetKeyCode()
            if char == wx.WXK_TAB then
                -- indent
                ctrl:WriteText('    ')
            elseif char == wx.WXK_RETURN or char == wx.WXK_NUMPAD_ENTER then
                -- return and indent
                ctrl:WriteText(
                    '\n'..
                    string.rep('    ', ctrl.getIndent())
                )
            elseif char == wx.WXK_BACK then
                local text = ctrl:GetRange(ctrl.getLineStart(), ctrl:GetInsertionPoint())
                local spaces = text:match('^( *)$')
                if spaces and #spaces ~= 0 then
                    local indent = math.floor(#spaces / 4)
                    local extra = #spaces - 4*indent
                    if extra == 0 then extra = 4 end
                    ctrl:Remove(ctrl:GetInsertionPoint() - extra, ctrl:GetInsertionPoint())
                else
                    evt:Skip()
                end
            else
                evt:Skip()
            end
        end
    )
    return ctrl
end

-- The Dialog
local function CreateDialog()
    local dlg = wx.wxDialog(
        xword.frame, wx.wxID_ANY, "Script tester",
        wx.wxDefaultPosition, wx.wxDefaultSize,
        wx.wxDEFAULT_DIALOG_STYLE + wx.wxRESIZE_BORDER
    )
    -- Controls
    dlg.mainsplitter = wx.wxSplitterWindow(dlg, wx.wxID_ANY)
    dlg.mainsplitter:SetMinimumPaneSize(40)
        dlg.textsplitter = wx.wxSplitterWindow(dlg.mainsplitter, wx.wxID_ANY)
        dlg.textsplitter:SetMinimumPaneSize(40)
            dlg.interpreter = createInterpreter(dlg.textsplitter, dlg)
            dlg.textCtrl = createTextCtrl(dlg.textsplitter, dlg)
        dlg.errorctrl = wx.wxTextCtrl(
            dlg.mainsplitter, wx.wxID_ANY,
            "Output console\n",
            wx.wxDefaultPosition, wx.wxDefaultSize,
            wx.wxTE_MULTILINE + wx.wxTE_DONTWRAP
        )
        dlg.errorctrl:SetFont(fixedWidthFont)
    dlg.runButton = wx.wxButton(dlg, wx.wxID_ANY, "Run Script")
    dlg.loadVarsButton = wx.wxButton(dlg, wx.wxID_ANY, "Init Debug Variables")
    dlg.treeButton = wx.wxButton(dlg, wx.wxID_ANY, "Examine Table")
    dlg.registryButton = wx.wxButton(dlg, wx.wxID_ANY, "Examine Registry")
    dlg.userMethodsButton = wx.wxButton(dlg, wx.wxID_ANY, "User-defined Variables")
    dlg.windowButton = wx.wxButton(dlg, wx.wxID_ANY, "Find a Window")

    -- Layout
    local mainSizer = wx.wxBoxSizer(wx.wxVERTICAL)
        dlg.mainsplitter:SplitHorizontally(dlg.textsplitter, dlg.errorctrl, -100)
        dlg.textsplitter:SplitVertically(dlg.interpreter, dlg.textCtrl)
        mainSizer:Add(dlg.mainsplitter, 1, wx.wxEXPAND + wx.wxALL, 5)
        local buttonSizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
            buttonSizer:Add(dlg.runButton,   0, wx.wxEXPAND + wx.wxALL, 5)
            buttonSizer:Add(dlg.loadVarsButton, 0, wx.wxEXPAND + wx.wxALL, 5)
            buttonSizer:Add(dlg.treeButton, 0, wx.wxEXPAND + wx.wxALL, 5)
            buttonSizer:Add(dlg.registryButton, 0, wx.wxEXPAND + wx.wxALL, 5)
            buttonSizer:Add(dlg.userMethodsButton, 0, wx.wxEXPAND + wx.wxALL, 5)
            buttonSizer:Add(dlg.windowButton, 0, wx.wxEXPAND + wx.wxALL, 5)
        mainSizer:Add(buttonSizer, 0, wx.wxEXPAND + wx.wxALL, 5)
    dlg:SetSizerAndFit(mainSizer)

    -- Printing functions
    --[[
    local realprint = print
    function print(...)
        dlg.errorctrl:AppendText(table2String(arg) .. '\n')
    end
    ]]

    function dlg.printResult(results)
        dlg.print("\n")
        local success, err = pcall(function()
            -- If results has one item, store it in _
            -- Otherwise, store the whole table in _
            if #results == 1 then
                local result = results[1]
                _G['_'] = result
                if type(result) == "table" then
                    dlg.printTable(result)
                else
                    dlg.printLine(result)
                end
            else
                _G['_'] = results
                for _, v in ipairs(results) do
                    dlg.print(v)
                    dlg.print('\t')
                end
                dlg.print('\n')
            end
        end)
        if not success then
            dlg.printError(err)
            dlg.print('\n')
        end
    end

    function dlg.print(msg)
        wx.wxSafeYield()
        if dlg.STOP_PRINT then
            dlg.STOP_PRINT = false
            error("Ctrl+C pressed")
        else
            dlg.interpreter:AppendText(tostring(msg))
        end
    end

    function dlg.printLine(msg)
        return dlg.print(tostring(msg)..'\n')
    end

    dlg.tableDepth = 5
    function dlg.printTable(t, indent, prev_tables)
        local indent = indent or 1
        dlg.printLine(string.rep(" ", 4*(indent-1)) .. tostring(t))
        if indent > dlg.tableDepth and dlg.tableDepth ~= -1 then return end

        local prev_tables = prev_tables or {}
        prev_tables[t] = true

        -- Find the longest key
        local maxKey = 0
        for k,v in pairs(t) do
            local len = #tostring(k)
            if len > maxKey then
                maxKey = len
            end
        end

        dlg.printLine(string.rep(" ", 4*(indent-1)) .. "{")
        for k,v in pairs(t) do
            if type(v) == 'table' and not prev_tables[v] then
                dlg.print(string.rep(" ", 4*indent) ..
                          tostring(k) .. ":" .. string.rep(" ", maxKey - #tostring(k) + 2))
                dlg.printTable(v, indent+1, prev_tables)
            else
                dlg.printLine(string.rep(" ", 4*indent) ..
                              tostring(k) .. ":" .. string.rep(" ", maxKey - #tostring(k) + 2) ..
                              tostring(v))
            end
        end
        dlg.printLine(string.rep("    ", indent-1) .. "}")
    end

    function dlg.printError(msg)
        dlg.errorctrl:AppendText(tostring(msg) .. '\n')
    end


    function dlg.execString(text)
        local func
        local msg
        func, msg = loadstring('return ' .. text)
        if not func then
            func, msg = loadstring(text)
        end
        if not func then
            dlg.printError(msg)
            dlg.print('\n')
        else
            local result = { pcall(func) }
            local success = result[1]
            table.remove(result, 1)
            if success then
                if #result > 0 then
                    dlg.printResult(result)
                else
                    dlg.print("\n")
                end
            else
                dlg.printError(result[1])
                dlg.print('\n')
            end
        end
    end


    -- Events
    dlg:Connect(dlg.loadVarsButton:GetId(),
                wx.wxEVT_COMMAND_BUTTON_CLICKED,
        -- -------------------------------------------------------------------
        -- XWord Debug Variables
        -- -------------------------------------------------------------------
        function(evt)
            local function pvar(name, t)
                local var = (t or _G)[name]
                if type(var) == "table" then
                    dlg.print(name.." = ")
                    dlg.printTable(var)
                else
                    dlg.printLine(name.." = "..tostring(var))
                end
            end

            dlg.printLine("")

            frame = xword.frame
            pvar("frame")

            p = frame.Puzzle
            pvar("p")

            g = p.Grid
            pvar("g")

            mgr = wxaui.wxAuiManager.GetManager(frame.Children:Item(0):GetData():DynamicCast("wxWindow"))
            pvar("mgr")

            -- This is mostly for pretty printing
            panes = {}
            local mgrpanes = mgr:GetAllPanes()
            for i=0, mgrpanes.Count-1 do
                local pane = mgrpanes:Item(i)
                panes[pane.name] = pane
            end
            pvar("panes")

            -- Here's the real action:
            -- panes.Name -> mgr:GetPane(Name)
            panes = {
                __names = tablex.pairmap(function(k,v) return k, k:lower() end,
                                         panes),
                __mgr = mgr
            }
            local mt = {}
            function mt:__index(name)
                if self.__names[name:lower()] then
                    return self.__mgr:GetPane(self.__names[name:lower()])
                end
            end
            setmetatable(panes, mt)

            dlg.printLine("")
            dlg.interpreter.showPrompt()
        end)

    dlg:Connect(dlg.runButton:GetId(),
                wx.wxEVT_COMMAND_BUTTON_CLICKED,
                function(evt)
                    dlg.execString(dlg.textCtrl:GetValue())
                end)

    local function eval(str)
        setfenv(0, _G)
        local func, res = loadstring('return ' .. str)
        if not func then
            func, res = loadstring(str)
        end
        if not func then
            xword.Error(res)
        else
            local success
            success, res = pcall(func)
            if not success then
                xword.Error(res)
            end
            return res
        end
    end

    local function showTree(str)
        if str ~= "" then
            eval(string.format("tableDialog(%s, %q):Show()", str, str))
        end
    end

    dlg:Connect(dlg.treeButton:GetId(),
                wx.wxEVT_COMMAND_BUTTON_CLICKED,
                function(evt)
                    showTree(wx.wxGetTextFromUser("Enter an expression that returns a table", "Table Dialog", "_G"))
                end)

    dlg:Connect(dlg.registryButton:GetId(),
                wx.wxEVT_COMMAND_BUTTON_CLICKED,
                function(evt)
                    showTree("debug.getregistry()")
                end)

    dlg:Connect(dlg.userMethodsButton:GetId(),
                wx.wxEVT_COMMAND_BUTTON_CLICKED,
                function(evt)
                    local str = wx.wxGetTextFromUser("Enter an expression that returns a wxLua object", "User-defined Methods", "interp")
                    if str ~= "" then
                        local obj = eval(str)
                        if obj then
                            local funcs = GetUserDefinedMethods(obj)
                            if funcs then
                                tableDialog(funcs, str):Show()
                            else
                                xword.Message("No user defined methods")
                            end
                        end
                    end
                end)

    dlg:Connect(wx.wxEVT_CLOSE_WINDOW,
                function(evt)
                    dlg:Destroy()
                    interp = nil
                    --print = realprint
                end)

    -- Select a window
    dlg:Connect(dlg.windowButton:GetId(), wx.wxEVT_COMMAND_BUTTON_CLICKED,
        function(evt)
            dlg:CaptureMouse()

            local last_window = nil

            local function motion(evt)
                local window = wx.wxFindWindowAtPointer(evt:GetPosition())
                if window and window:IsTopLevel() then
                    window = nil
                end
                if last_window ~= window then
                    if last_window then
                        last_window:Refresh()
                    end
                    last_window = window
                end
                local dc = wx.wxScreenDC()
                if window then
                    dc:SetPen(wx.wxPen(wx.wxRED, 3, wx.wxSOLID))
                    dc:SetBrush(wx.wxTRANSPARENT_BRUSH)
                    local pos = window:GetScreenPosition()
                    local size = window:GetSize()
                    dc:DrawRectangle(pos.X+1, pos.Y+1, size.Width-2, size.Height-2)
                end
                dc:delete()
            end

            local function leftdown(evt)
                local success
                local window = wx.wxFindWindowAtPointer(evt:GetPosition())
                if window then
                    -- Try to convert this window to its classname
                    dlg.printError(window.ClassInfo:IsDynamic())
                    success, w = pcall(function() return window:DynamicCast(window.ClassInfo.ClassName) end)
                    if not success then w = window end
                    dlg.printError("Window is available as variable 'w'")
                    dlg.printError("w = " .. tostring(w))
                    -- Erase the red line
                    window:Refresh()
                end
                dlg:Disconnect(wx.wxEVT_LEFT_DOWN)
                dlg:Disconnect(wx.wxEVT_MOTION)
                dlg:ReleaseMouse()
            end

            dlg:Connect(wx.wxEVT_LEFT_DOWN, leftdown)
            dlg:Connect(wx.wxEVT_MOTION, motion)
        end)

    dlg:SetSize(750, 500)
    return dlg
end

local function init()
    xword.frame:AddMenuItem({'Debug'}, 'Interpreter\tCtrl+I',
        function(evt)
            CreateDialog():Show()
        end
    )
end

init()
