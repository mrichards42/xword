local join = require 'pl.path'.join
local deepcopy = require 'pl.tablex'.deepcopy
local clear = require 'pl.tablex'.clear

download.puzzle_directory = xword.userdatadir.."\\puzzles"
download.separate_directories = true
download.disabled = {
    "NY Times Premium",
    "Newsday",
    "Universal",
}

-- Load the config file
local success, config = pcall(dofile, join(xword.configdir, 'download', 'config.lua'))
if success and config then
    for _, name in ipairs({'puzzle_directory', 'separate_directories'}) do
        if config[name] ~= nil then
            download[name] = config[name]
        end
    end

    local dl_list = config.disabled or download.disabled
    download.disabled = {}
    for o, name in ipairs(dl_list) do
        download.disabled[name] = true
    end

    -- Add download sources
    for _, puzzle in ipairs(config.added or {}) do
        download.puzzles:insert(puzzle)
    end
    -- Changed sources
    for key, data in pairs(config.changed or {}) do
        local puzzle = download.puzzles[key]
        local function update(puz1, puz2)
            -- Update nil values
            for _, k in ipairs(puz2._nil or {}) do
                puz1[k] = nil
            end
            puz2._nil = nil
            -- Copy puz2 to puz1
            for k,v in pairs(puz2) do
                if type(v) == 'table' then
                    if type(puz1[k]) ~= 'table' then
                        puz1[k] = {}
                    end
                    update(puz1[k], puz2[k])
                else
                    puz1[k] = v
                end
            end
        end
        if puzzle then
            update(puzzle, data)
        end
    end
    -- Adjust the order
    local oldorder = download.puzzles._order
    download.puzzles._order = {}
    for _, id in ipairs(config.order or {}) do
        -- Look for this id in the order.
        for i, puzid in ipairs(oldorder) do
            if puzid == id then
                table.insert(download.puzzles._order, id)
                table.remove(oldorder, i)
            end
        end
    end
    -- Add the rest of the puzzles in order
    for _, id in ipairs(oldorder) do
        table.insert(download.puzzles._order, id)
    end
end

-- Write config to a file
function download.save_config()
    local config = {
        puzzle_directory = download.puzzle_directory,
        separate_directories = download.separate_directories,
        disabled = {},
        added = {},
        changed = {},
        order = download.puzzles._order
    }
    for id, disabled in pairs(download.disabled or {}) do
        if disabled then
            table.insert(config.disabled, id)
        end
    end
    -- Figure out what has changed from the defaults
    local defaults = download.get_default_puzzles()
    for _, puzzle in download.puzzles:iter() do
        local default = defaults[puzzle.id]
        if default then
            local function get_changes(puz1, puz2)
                local changes = { _nil = {} }
                local changed = false
                -- Check for changed values
                for k,v in pairs(puz1) do
                    if puz2[k] ~= v then
                        if type(puz1[k]) == 'table' and type(puz2[k]) == 'table' then
                            changes[k] = get_changes(puz1[k], puz2[k])
                            changed = changed or changes[k]
                        else
                            changes[k] = v
                            changed = true
                        end
                    end
                end
                -- Check for removed / nil values
                for k,v in pairs(puz2) do
                    if puz1[k] == nil then
                        table.insert(changes._nil, k)
                    end
                end
                if #changes._nil == 0 then
                    changes._nil = nil
                else
                    changed = true
                end
                if changed then return changes end
            end
            config.changed[puzzle.id] = get_changes(puzzle, default)
        else
            table.insert(config.added, puzzle)
        end
    end

    pl.dir.makepath(join(xword.configdir, 'download'))
    serialize.pdump(config, join(xword.configdir, 'download', 'config.lua'))
end

-- Helper function for download directory
function download.sanitize_name(text)
    local text = text:gsub('[?<>:*|"\']', ""):gsub("%s", "_")
    return text
end

-- ============================================================================
-- GUI
-- ============================================================================

require 'luacurl'
local ArrowButton = require 'download.arrow_button'
local TextButton = require 'download.text_button'
local BmpButton = require 'download.bmp_button'
local bmp = require 'download.bmp'


function scrolled_panel(parent)
    local scroller = wx.wxScrolledWindow(parent, wx.wxID_ANY)
    scroller:SetWindowStyle(wx.wxBORDER_DOUBLE)
    scroller:SetScrollRate(10, 10)

    function scroller:update_scrollbars()
        self.panel:Layout()
        self:FitInside()
        self:Refresh()
    end

    -- This panel preserves tab behavior
    local psizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    local panel = wx.wxPanel(scroller, wx.wxID_ANY)
    psizer:Add(panel, 1, wx.wxEXPAND)
    scroller:SetSizer(psizer)

    scroller.panel = panel
    return scroller
end

function collapse_panel(parent, label)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    panel:SetSizer(sizer)

    -- Header
    local header = wx.wxBoxSizer(wx.wxHORIZONTAL)
    sizer:Add(header, 0, wx.wxEXPAND + wx.wxBOTTOM, 5)

    local arrow = ArrowButton(panel, wx.wxID_ANY)
    header:Add(arrow, 0, wx.wxALIGN_CENTER_VERTICAL + wx.wxRIGHT, 5)
    local text = TextButton(panel, wx.wxID_ANY, label)
    header:Add(text, 0, wx.wxALIGN_CENTER_VERTICAL)

    -- Collapsing
    local inside_panel
    local function update_panel()
        if inside_panel then
            sizer:Show(inside_panel, arrow:IsExpanded())
            sizer:Layout()
            parent.Parent:Fit()
        end
    end

    arrow:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, update_panel)
    
    text:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        arrow:Toggle()
        update_panel()
    end)

    -- Public stuff
    panel.header = header
    panel.sizer = sizer
    function panel:set_panel(p)
        inside_panel = p
        sizer:Add(inside_panel, 1, wx.wxEXPAND)
        update_panel()
    end

    return panel
end

-- ----------------------------------------------------------------------------
-- Advanced Options / New source dialog
-- ----------------------------------------------------------------------------

function get_curl_options_panel(parent, puzzle)
    local panel = collapse_panel(parent, "Download Options")

    -- Header additions
    panel.header:AddStretchSpacer()
    local link = TextButton(panel, wx.wxID_ANY, "(help)")
    link:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
        wx.wxLaunchDefaultBrowser(
            "http://curl.haxx.se/libcurl/c/curl_easy_setopt.html")
    end)
    link.ForegroundColour = wx.wxBLUE
    panel.header:Add(link)

    -- Options
    local scroller = scrolled_panel(panel, wx.wxID_ANY)
    panel:set_panel(scroller)

    local grid = wx.wxFlexGridSizer(0, 3, 5, 5)
    grid:AddGrowableCol(1)
    scroller.panel:SetSizer(grid)

    local curl_opts = {}
    local opt_names = {}
    for k, v in pairs(curl) do
        if k:sub(1,4) == "OPT_" then
            table.insert(curl_opts, k:sub(5))
            opt_names[v] = k:sub(5)
        end
    end
    table.sort(curl_opts)

    local opts = {}

    local function add_option(name, value)
        local opt = wx.wxChoice(
            scroller.panel, wx.wxID_ANY,
            wx.wxDefaultPosition, wx.wxDefaultSize, curl_opts)
        opt.StringSelection = opt_names[name] or ''
        local text = wx.wxTextCtrl(scroller.panel, wx.wxID_ANY, value or '')
        local remove_button = BmpButton(scroller.panel, wx.wxID_ANY, bmp.remove)
        remove_button.ToolTip = wx.wxToolTip("Remove option")
        grid:Insert(#opts * 3, opt)
        grid:Insert(#opts * 3 + 1, text, 1, wx.wxEXPAND)
        grid:Insert(#opts * 3 + 2, remove_button, 0, wx.wxALIGN_CENTER_VERTICAL)
        table.insert(opts, {opt, text})

        -- Remove event
        remove_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
            for i, ctrls in ipairs(opts) do
                if ctrls[1] == opt then
                    table.remove(opts, i)
                    grid:Detach((i-1) * 3)
                    grid:Detach((i-1) * 3)
                    grid:Detach((i-1) * 3)
                    opt:Destroy()
                    text:Destroy()
                    remove_button:Destroy()
                    scroller:update_scrollbars()
                    break
                end
            end
        end)
    end

    -- The add button
    grid:AddSpacer(0)
    grid:AddSpacer(0)
    local add_button = BmpButton(scroller.panel, wx.wxID_ANY, bmp.add)
    grid:Add(add_button, 0, wx.wxALIGN_CENTER_VERTICAL)
    add_button.ToolTip = wx.wxToolTip("Add option")
    add_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        scroller:Freeze()
        add_option()
        scroller:update_scrollbars()
        scroller:Thaw()
    end)

    scroller.MinSize = wx.wxSize(-1, 100)

    function panel:apply()
        if not puzzle.curlopts then
            puzzle.curlopts = {}
        end
        clear(puzzle.curlopts)
        for _, ctrls in ipairs(opts) do
            puzzle.curlopts[curl['OPT_' .. ctrls[1].StringSelection]] = ctrls[2].Value
        end
        if #opts == 0 then
            puzzle.curlopts = nil
        end
    end

    function panel:update_puzzle(p)
        -- Clear the grid sizer
        for i=0,#opts*3-1 do
            local window = grid:GetItem(0).Window
            grid:Detach(0)
            window:Destroy()
        end
        clear(opts)
        for k, v in pairs(p.curlopts or {}) do
            if type(v) == 'string' and opt_names[k] then
                add_option(k, v)
            end
        end
        scroller:update_scrollbars()
    end

    panel:update_puzzle(puzzle)

    return panel
end

function get_download_fields_panel(parent, puzzle)
    local panel = collapse_panel(parent, "Custom User Options")

    -- Fields
    local scroller = scrolled_panel(panel, wx.wxID_ANY)
    panel:set_panel(scroller)

    local grid = wx.wxFlexGridSizer(0, 2, 5, 5)
    grid:AddGrowableCol(0)
    scroller.panel:SetSizer(grid)

    local fields = {}

    local function add_field(value)
        local text = wx.wxTextCtrl(scroller.panel, wx.wxID_ANY, value or '')
        local remove_button = BmpButton(scroller.panel, wx.wxID_ANY, bmp.remove)
        remove_button.ToolTip = wx.wxToolTip("Remove field")
        grid:Insert(#fields * 2, text, 1, wx.wxEXPAND)
        grid:Insert(#fields * 2 + 1, remove_button, 0, wx.wxALIGN_CENTER_VERTICAL)
        table.insert(fields, text)

        -- Remove event
        remove_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
            for i, ctrl in ipairs(fields) do
                if ctrl == text then
                    table.remove(fields, i)
                    grid:Detach((i-1) * 2)
                    grid:Detach((i-1) * 2)
                    text:Destroy()
                    remove_button:Destroy()
                    scroller:update_scrollbars()
                    break
                end
            end
        end)
    end

    -- The add button
    grid:AddSpacer(0)
    local add_button = BmpButton(scroller.panel, wx.wxID_ANY, bmp.add)
    grid:Add(add_button, 0, wx.wxALIGN_CENTER_VERTICAL)
    add_button.ToolTip = wx.wxToolTip("Add field")
    add_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        scroller:Freeze()
        add_field()
        scroller:update_scrollbars()
        scroller:Thaw()
    end)

    scroller.MinSize = wx.wxSize(-1, 100)

    function panel:apply()
        if not puzzle.fields then
            puzzle.fields = {}
        end
        clear(puzzle.fields)
        for _, ctrl in ipairs(fields) do
            local value = ctrl.Value
            if value ~= '' then
                table.insert(puzzle.fields, value)
            end
        end
        if #puzzle.fields == 0 then
            puzzle.fields = nil
        end
    end

    function panel:update_puzzle(p)
        -- Clear the grid sizer
        for i=0,#fields*2-1 do
            local window = grid:GetItem(0).Window
            grid:Detach(0)
            window:Destroy()
        end
        clear(fields)
        for _, name in ipairs(p.fields or {}) do
            add_field(name)
        end
        scroller:update_scrollbars()
    end

    panel:update_puzzle(puzzle)

    return panel
end

function get_download_function_panel(parent, puzzle)
    local panel = collapse_panel(parent, "Custom Download Function")

    local font = wx.wxFont(8, wx.wxFONTFAMILY_MODERN, wx.wxFONTSTYLE_NORMAL,
                           wx.wxFONTWEIGHT_NORMAL, false)

    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)

    local st = wx.wxStaticText(panel, wx.wxID_ANY, "function (puzzle)")
    st.Font = font
    sizer:Add(st, 0, wx.wxLEFT, 10)

    local text = wx.wxTextCtrl(panel, wx.wxID_ANY, puzzle.func or '',
        wx.wxDefaultPosition, wx.wxDefaultSize,
        wx.wxTE_MULTILINE + wx.wxHSCROLL)
    text.Font = font
    text.MinSize = wx.wxSize(-1, 100)
    sizer:Add(text, 1, wx.wxEXPAND + wx.wxLEFT, 20)

    local st = wx.wxStaticText(panel, wx.wxID_ANY, "end")
    st.Font = font
    sizer:Add(st, 0, wx.wxLEFT, 10)

    panel:set_panel(sizer)

    function panel:apply()
        local value = text.Value
        if value:match('^%s*$') then
            puzzle.func = nil
        else
            puzzle.func = value
        end
    end

    function panel:update_puzzle(p)
        text.Value = p.func or ''
    end

    return panel
end

function get_advanced_options_panel(parent, puzzle)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)

    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    panel:SetSizer(sizer)

    local grid = wx.wxFlexGridSizer(0, 2, 5, 5)
    grid:AddGrowableCol(1)
    sizer:Add(grid, 0, wx.wxEXPAND + wx.wxBOTTOM, 5)

    local name = wx.wxTextCtrl(panel, wx.wxID_ANY, puzzle.name)
    grid:Add(wx.wxStaticText(panel, wx.wxID_ANY, "Name:"), 0, wx.wxALIGN_CENTER_VERTICAL)
    grid:Add(name, 1, wx.wxEXPAND)

    local url = wx.wxTextCtrl(panel, wx.wxID_ANY, puzzle.url or '')
    grid:Add(wx.wxStaticText(panel, wx.wxID_ANY, "URL:"), 0, wx.wxALIGN_CENTER_VERTICAL)
    grid:Add(url, 1, wx.wxEXPAND)

    local filename = wx.wxTextCtrl(panel, wx.wxID_ANY, puzzle.filename or '')
    grid:Add(wx.wxStaticText(panel, wx.wxID_ANY, "Local filename:"), 0, wx.wxALIGN_CENTER_VERTICAL)
    grid:Add(filename, 1, wx.wxEXPAND)

    local directoryname = wx.wxTextCtrl(panel, wx.wxID_ANY, download.sanitize_name(puzzle.directoryname or puzzle.name))
    grid:Add(wx.wxStaticText(panel, wx.wxID_ANY, "Subdirectory:"), 0, wx.wxALIGN_CENTER_VERTICAL)
    grid:Add(directoryname, 1, wx.wxEXPAND)

    local daysizer = wx.wxStaticBoxSizer(wx.wxHORIZONTAL, panel, "Days Available")
    sizer:Add(daysizer, 0)
    local days = {}
    local daygrid = wx.wxGridSizer(1, 0, 5, 5)
    daysizer:Add(daygrid, 1, wx.wxEXPAND + wx.wxALL, 5)
    for i, label in ipairs({'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'}) do
        local ctrl = wx.wxCheckBox(panel, wx.wxID_ANY, label)
        ctrl.Value = puzzle.days[i]
        table.insert(days, ctrl)
        daygrid:Add(ctrl, 1, wx.wxEXPAND)
    end

    -- cURL options
    local curlopts = get_curl_options_panel(panel, puzzle)
    sizer:Add(curlopts, 0, wx.wxEXPAND + wx.wxTOP, 10)

    -- fields
    local fields = get_download_fields_panel(panel, puzzle)
    sizer:Add(fields, 0, wx.wxEXPAND + wx.wxTOP, 10)

    -- custom function
    local func = get_download_function_panel(panel, puzzle)
    sizer:Add(func, 0, wx.wxEXPAND + wx.wxTOP, 10)

    function panel:apply()
        puzzle.name = name.Value
        puzzle.filename = filename.Value
        puzzle.url = url.Value
        if directoryname.Value ~= download.sanitize_name(puzzle.name) then
            puzzle.directoryname = directoryname.Value
        else
            puzzle.directoryname = nil
        end
        for i, ctrl in ipairs(days) do
            puzzle.days[i] = ctrl.Value
        end
        curlopts:apply()
        fields:apply()
        func:apply()
    end

    function panel:update_puzzle(p)
        name.Value = p.name
        filename.Value = p.filename
        url.Value = p.url
        directoryname.Value = p.directoryname or download.sanitize_name(p.name)
        for i, ctrl in ipairs(days) do
            ctrl.Value = p.days[i]
        end
        curlopts:update_puzzle(p)
        fields:update_puzzle(p)
        func:update_puzzle(p)
    end

    return panel
end

function show_advanced_options_dialog(parent, puzzle, title)
    local dlg = wx.wxDialog(parent, wx.wxID_ANY, title or "Advanced Options")

    -- Create a new puzzle
    if puzzle == nil or not puzzle.name then
        puzzle = { name = '', filename = '', url = '',
                   days = { false, false, false, false, false, false, false} }
    end

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    dlg:SetSizer(sizer)
    local panel = get_advanced_options_panel(dlg, puzzle)
    panel.MinSize = wx.wxSize(400, -1)
    sizer:Add(panel, 1, wx.wxEXPAND + wx.wxALL, 5)

    local buttons = dlg:CreateButtonSizer(wx.wxOK + wx.wxCANCEL)
    sizer:Add(buttons, 0, wx.wxALL, 5)

    local reset_button = wx.wxButton(dlg, wx.wxID_ANY, "Reset to Defaults")
    buttons:Add(reset_button)
    reset_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        local p = download.get_default_puzzles()[puzzle.id]
        if p then
            panel:update_puzzle(p)
        end
    end)

    dlg:Fit()
    dlg.MinSize = dlg.Size
    dlg:Center()

    if dlg:ShowModal() == wx.wxID_OK then
        panel:apply()
        dlg:Destroy()
        return puzzle
    end
    dlg:Destroy()
end

-- ----------------------------------------------------------------------------
-- Config dialog
-- ----------------------------------------------------------------------------

local function get_download_fields(parent, puzzle)
    -- If a source has options, add them to a 'fields' table (see NYT Premium
    -- for example)
    local function field_key(name)
        return name:gsub('%s', '_'):lower()
    end

    local fields = {}
    for _, name in ipairs(puzzle.fields or {}) do
        table.insert(fields, name)
        fields[name] = true
    end

    local scroller = scrolled_panel(parent)

    local border = wx.wxBoxSizer(wx.wxVERTICAL)
    scroller.panel:SetSizer(border)

    local enabled = wx.wxCheckBox(scroller.panel, wx.wxID_ANY, "Enabled")
    enabled.Value = not download.disabled[puzzle.id]
    border:Add(enabled, 0, wx.wxEXPAND + wx.wxLEFT + wx.wxRIGHT + wx.wxTOP, 5)

    local sizer = wx.wxFlexGridSizer(0, 2, 5,5)
    border:Add(sizer, 1, wx.wxEXPAND + wx.wxALL, 5)
    sizer:AddGrowableCol(1)

    for _, name in ipairs(fields) do
        sizer:Add(wx.wxStaticText(scroller.panel, wx.wxID_ANY, name), 0, wx.wxALIGN_CENTER_VERTICAL)
        fields[name] = wx.wxTextCtrl(scroller.panel, wx.wxID_ANY, puzzle[field_key(name)] or '')
        sizer:Add(fields[name], 1, wx.wxEXPAND)
    end

    function scroller:apply()
        for _, name in ipairs(fields) do
            local ctrl = fields[name]
            puzzle[field_key(name)] = ctrl.Value
        end
        download.disabled[puzzle.id] = not enabled.Value
    end

    return scroller
end

local function get_download_options_panel(parent, puzzle)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    panel:SetSizer(sizer)

    local options = get_download_fields(panel, puzzle)
    sizer:Add(options, 1, wx.wxEXPAND)

    local advanced = wx.wxButton(panel, wx.wxID_ANY, "Advanced Options")
    sizer:Add(advanced, 0, wx.wxTOP + wx.wxALIGN_CENTER,  5)

    advanced:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
        show_advanced_options_dialog(panel, puzzle)
        parent.list:update_sources()
        parent:update_details()
    end)

    function panel:apply()
        options:apply()
    end
    return panel
end


function download.get_config_panel(parent)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)

    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    panel:SetSizer(sizer)

    local sizer1 = wx.wxBoxSizer(wx.wxHORIZONTAL)
    sizer:Add(sizer1, 0, wx.wxEXPAND + wx.wxBOTTOM, 5)

    local puzzle_directory = wx.wxDirPickerCtrl(panel, wx.wxID_ANY, download.puzzle_directory)
    sizer1:Add(wx.wxStaticText(panel, wx.wxID_ANY, "Download Directory:"), 0, wx.wxALIGN_CENTER)
    sizer1:Add(puzzle_directory, 1, wx.wxEXPAND)

    local separate_directories = wx.wxRadioBox(
        panel, wx.wxID_ANY, "Download puzzles to:",
        wx.wxDefaultPosition, wx.wxDefaultSize,
        {"One directory", "Directories by source"}, 2
    )
    separate_directories.Selection = download.separate_directories and 1 or 0
    sizer:Add(separate_directories)

    -- Puzzle Sources
    local srcsizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    sizer:Add(srcsizer, 1, wx.wxEXPAND + wx.wxTOP, 5)

    local listsizer = wx.wxStaticBoxSizer(wx.wxVERTICAL, panel, "Puzzle sources")
    srcsizer:Add(listsizer, 0, wx.wxEXPAND)

    local puzzle_list = wx.wxListBox(panel, wx.wxID_ANY, wx.wxDefaultPosition,
                                     wx.wxSize(-1, 150))
    panel.list = puzzle_list
    listsizer:Add(puzzle_list, 1, wx.wxEXPAND)
    
    local puzzles = deepcopy(download.puzzles)
    function puzzle_list:update_sources()
        local names = {}
        for key, puzzle in puzzles:iter() do
            table.insert(names, puzzle.name)
        end
        local selection = puzzle_list.Selection
        puzzle_list:Set(names)
        puzzle_list.Selection = selection
    end

    -- List buttons
    local buttonsizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    listsizer:Add(buttonsizer, 0, wx.wxALL + wx.wxALIGN_RIGHT, 5)

    local btn_up = BmpButton(panel, wx.wxID_ANY, bmp.up)
    buttonsizer:Add(btn_up, 0, wx.wxLEFT, 5)
    btn_up.ToolTip = wx.wxToolTip("Move up in list")
    btn_up:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        local idx = puzzle_list.Selection + 1
        if idx > 1 then
            local id = table.remove(puzzles._order, idx)
            table.insert(puzzles._order, idx-1, id)
            puzzle_list:Delete(idx - 1)
            puzzle_list:Insert(id, idx - 2)
            puzzle_list.Selection = idx - 2
        end
    end)

    local btn_down = BmpButton(panel, wx.wxID_ANY, bmp.down)
    buttonsizer:Add(btn_down, 0, wx.wxLEFT, 5)
    btn_down.ToolTip = wx.wxToolTip("Move down in list")
    btn_down:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        local idx = puzzle_list.Selection + 1
        if idx < #puzzles._order then
            local id = table.remove(puzzles._order, idx)
            table.insert(puzzles._order, idx+1, id)
            puzzle_list:Delete(idx - 1)
            puzzle_list:Insert(id, idx)
            puzzle_list.Selection = idx
        end
    end)

    local btn_remove = BmpButton(panel, wx.wxID_ANY, bmp.remove)
    buttonsizer:Add(btn_remove, 0, wx.wxLEFT, 5)
    btn_remove.ToolTip = wx.wxToolTip("Remove from list")
    btn_remove:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        local idx = puzzle_list.Selection + 1
        if idx > 0 then
            local id = puzzles._order[idx]
            -- Only remove puzzles that we created
            if download.get_default_puzzles()[id] == nil then
                puzzles:remove(id)
                puzzle_list:Delete(idx - 1)
                if idx == 1 then
                    puzzle_list.Selection = idx - 1
                else
                    puzzle_list.Selection = idx - 2
                end
            end
        end
    end)


    local btn_add = BmpButton(panel, wx.wxID_ANY, bmp.add)
    buttonsizer:Add(btn_add, 0, wx.wxLEFT, 5)
    btn_add.ToolTip = wx.wxToolTip("Add a new source")
    btn_add:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        local puzzle = show_advanced_options_dialog(panel, "New Source")
        if puzzle then
            puzzles:insert(puzzle)
            local selection = puzzle_list.Selection
            puzzle_list:update_sources()
            puzzle_list.Selection = selection
        end
    end)

    -- Details panel
    local detailsizer = wx.wxStaticBoxSizer(wx.wxVERTICAL, panel, "Configuration")
    srcsizer:Add(detailsizer, 1, wx.wxEXPAND + wx.wxLEFT, 5)
    local details

    local text = wx.wxStaticText(panel, wx.wxID_ANY, "Select a Puzzle",
                                 wx.wxDefaultPosition, wx.wxDefaultSize,
                                 wx.wxALIGN_CENTER)
    detailsizer:Add(text, 1, wx.wxALIGN_CENTER)

    function panel:update_details()
        if details then
            details:apply()
            detailsizer:Detach(details)
            details:Destroy()
        end
        if puzzle_list.Selection == -1 then
            detailsizer:Show(text, true)
            text.Label = "Select a Puzzle"
        else
            detailsizer:Show(text, false)
            details = get_download_options_panel(panel, puzzles:get(puzzle_list.Selection + 1))
            detailsizer:Add(details, 1, wx.wxEXPAND + wx.wxLEFT, 5)
            detailsizer:Show(details, true)
        end
        detailsizer:Layout()
    end

    puzzle_list:Connect(wx.wxEVT_COMMAND_LISTBOX_SELECTED, function(evt)
        panel:update_details()
        evt:Skip()
    end)

    puzzle_list:Connect(wx.wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, function(evt)
        evt:Skip()
        if puzzle_list.Selection ~= -1 then
            show_advanced_options_dialog(panel, puzzles:get(puzzle_list.Selection + 1))
            puzzle_list:update_sources()
            panel:update_details()
        end
    end)

    panel.MinSize = wx.wxSize(500, -1)

    function panel:apply()
        download.puzzle_directory = puzzle_directory.Path
        download.separate_directories = separate_directories.Selection == 1
        if details then details:apply() end
        download.puzzles = deepcopy(puzzles)
    end
    
    puzzle_list:update_sources()

    return panel
end

function download.show_config_dialog(parent)
    xword.showerrors = false
    local dlg = wx.wxDialog(parent or xword.frame, wx.wxID_ANY, "Download Config",
                            wx.wxDefaultPosition, wx.wxSize(500,500))

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    dlg:SetSizer(sizer)
    local panel = download.get_config_panel(dlg)
    sizer:Add(panel, 1, wx.wxEXPAND + wx.wxALL, 5)
    local buttons = dlg:CreateButtonSizer(wx.wxOK + wx.wxCANCEL)
    sizer:Add(buttons, 0, wx.wxALL, 5)

    dlg:Fit()

    dlg:Center()
    if dlg:ShowModal() == wx.wxID_OK then
        panel:apply()
        download.save_config()
    end
    dlg:Destroy()
end