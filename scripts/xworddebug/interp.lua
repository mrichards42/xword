-- ============================================================================
-- interp.lua
--     A wxLua script that functions as a very primitive shell.
-- ============================================================================


-- locals
local fixedWidthFont = wx.wxFont(8, wx.wxFONTFAMILY_MODERN,
                                 wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_NORMAL)

-- Make a string given a table
local function table2String(t)
    str = ''
    for i,val in ipairs(t) do
        val = tostring(val)
        str = str .. val
        if i < #t then
            str = str .. string.rep(' ', 20 - #val)
        end
    end
    return str
end


-- The Interpreter
local function createInterpreter(parent, dlg, id, value, pos, size, style)
    local id = id or wx.wxID_ANY
    local value = value or "Welcome to lua!"
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
    local showPrompt = function()
        interpreter:AppendText('\n' .. prompt)
    end

    local getLine = function()
        return interpreter:GetLineText(
            interpreter:GetNumberOfLines() - 1
        ):sub(#prompt)
    end

    showPrompt()

    interpreter:Connect(wx.wxID_ANY, wx.wxEVT_KEY_DOWN,
        function(evt)
            local char = evt:GetKeyCode()
            if char == wx.WXK_TAB then
                interpreter:WriteText('    ')
                wx.wxMessageBox("tab")
                evt:Skip(false)
                return
            -- Enter a command
            elseif char == wx.WXK_RETURN or char == wx.WXK_NUMPAD_ENTER then
                local text = getLine()
                table.insert(history, text)
                historyNum = #history + 1
                dlg.execString(text)
                showPrompt()
                evt:Skip(false)
                return
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
                return
            end
            
            evt:Skip()
        end)

    return interpreter
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
            dlg.textCtrl = wx.wxTextCtrl(
                dlg.textsplitter, wx.wxID_ANY,
                "-- Enter a function or other multi-line chunk here\n" ..
                "-- then press 'Run Script'\n",
                wx.wxDefaultPosition, wx.wxDefaultSize,
                wx.wxTE_MULTILINE + wx.wxTE_DONTWRAP + wx.wxTE_PROCESS_ENTER
            )
            dlg.textCtrl:SetFont(fixedWidthFont)
        dlg.errorctrl = wx.wxTextCtrl(
            dlg.mainsplitter, wx.wxID_ANY,
            "Output console\n",
            wx.wxDefaultPosition, wx.wxDefaultSize,
            wx.wxTE_MULTILINE + wx.wxTE_DONTWRAP
        )
        dlg.errorctrl:SetFont(fixedWidthFont)
    dlg.runButton = wx.wxButton(dlg, wx.wxID_ANY, "Run Script")
    dlg.closeButton = wx.wxButton(dlg, wx.wxID_ANY, "Done")

    -- Layout
    local mainSizer = wx.wxBoxSizer(wx.wxVERTICAL)
        dlg.mainsplitter:SplitHorizontally(dlg.textsplitter, dlg.errorctrl, -100)
        dlg.textsplitter:SplitVertically(dlg.interpreter, dlg.textCtrl)
        mainSizer:Add(dlg.mainsplitter, 1, wx.wxEXPAND + wx.wxALL, 5)
        local buttonSizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
            buttonSizer:Add(dlg.runButton,   0, wx.wxEXPAND + wx.wxALL, 5)
            buttonSizer:Add(dlg.closeButton, 0, wx.wxEXPAND + wx.wxALL, 5)
        mainSizer:Add(buttonSizer, 0, wx.wxEXPAND + wx.wxALL, 5)
    dlg:SetSizerAndFit(mainSizer)

    -- Printing functions
    local realprint = print
    function print(...)
        dlg.errorctrl:AppendText(table2String(arg) .. '\n')
    end

    dlg.printResult = function(results)
        dlg.interpreter:AppendText('\n')

        -- If results has one item, store it in _
        -- Otherwise, store the whole table in _
        if #results == 1 then
            _G['_'] = results[1]
        else
            _G['_'] = results
        end

        dlg.interpreter:AppendText(table2String(results))
    end

    dlg.printError = function(msg)
        dlg.errorctrl:AppendText(msg .. '\n')
    end


    dlg.execString = function(text)
        local func
        local msg
        func, msg = loadstring('return ' .. text)
        if not func then
            func, msg = loadstring(text)
        end
        if not func then
            dlg.printError(msg)
        else
            local result = { pcall(func) }
            local success = result[1]
            table.remove(result, 1)
            if success then
                if #result > 0 then dlg.printResult(result) end
            else
                dlg.printError(result[1])
            end
        end
    end


    -- Events
    dlg:Connect(dlg.closeButton:GetId(),
                wx.wxEVT_COMMAND_BUTTON_CLICKED,
                function(evt) dlg:Close() end)

    dlg:Connect(dlg.runButton:GetId(),
                wx.wxEVT_COMMAND_BUTTON_CLICKED,
                function(evt)
                    dlg.execString(dlg.textCtrl:GetValue())
                end)

    dlg:Connect(dlg:GetId(),
                 wx.wxEVT_CLOSE_WINDOW,
                 function(evt)
                    dlg:Destroy()
                    print = realprint
                 end)

    dlg:SetSize(750, 500)
    return dlg
end

local function init()
    xword.frame:AddMenuItem({'Tools', 'Debug'}, 'Interpreter',
        function(evt)
            local dlg = CreateDialog()
            dlg:Show()
        end
    )
end

init()
