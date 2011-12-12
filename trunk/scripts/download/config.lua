download.puzzle_directory = xword.userdatadir.."\\puzzles"
download.date_format = '%Y%m%d'
download.separate_directories = true
download.disabled = {}

-- Load the config file
local success, config = pcall(dofile, xword.configdir..'/download/config.lua')
if success and config then
    for _, name in ipairs({'puzzle_directory', 'date_format', 'separate_directories'}) do
        if config[name] ~= nil then
            download[name] = config[name]
        end
    end

    for _, id in ipairs(config.disabled or {}) do
        download.disabled[id] = true
    end

    -- Add download sources
    for _, puzzle in ipairs(config.add or {}) do
        table.insert(download.puzzles, puzzle)
    end
    -- Changed sources
    for _, data in ipairs(config.changed or {}) do
        if data.name then
            -- Look for this puzzle in the sources
            for i, puzzle in ipairs(download.puzzles) do
                if puzzle.name == data.name then
                    if data.newname then
                        puzzle.name = data.newname
                        data.newname = nil
                    end
                    -- Copy from data to puzzle
                    for k,v in pairs(data) do
                        puzzle[k] = v
                    end
                    break
                end
            end
        end
    end
end

-- Helper function for download directory
function download.sanitize_name(text)
    local text = text:gsub('[?<>:*|"\']', ""):gsub("%s", "_")
    return text
end

-- ============================================================================
-- GUI
-- ============================================================================

local deepcopy = require 'pl.tablex'.deepcopy
local clear = require 'pl.tablex'.clear

-- ----------------------------------------------------------------------------
-- Advanced Options / New source dialog
-- ----------------------------------------------------------------------------

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

    local url = wx.wxTextCtrl(panel, wx.wxID_ANY, puzzle.url)
    grid:Add(wx.wxStaticText(panel, wx.wxID_ANY, "URL:"), 0, wx.wxALIGN_CENTER_VERTICAL)
    grid:Add(url, 1, wx.wxEXPAND)

    local filename = wx.wxTextCtrl(panel, wx.wxID_ANY, puzzle.filename)
    grid:Add(wx.wxStaticText(panel, wx.wxID_ANY, "Local filename:"), 0, wx.wxALIGN_CENTER_VERTICAL)
    grid:Add(filename, 1, wx.wxEXPAND)

    local directoryname = wx.wxTextCtrl(panel, wx.wxID_ANY, download.sanitize_name(puzzle.directoryname or puzzle.name))
    grid:Add(wx.wxStaticText(panel, wx.wxID_ANY, "Subdirectory:"), 0, wx.wxALIGN_CENTER_VERTICAL)
    grid:Add(directoryname, 1, wx.wxEXPAND)

    local daysizer = wx.wxStaticBoxSizer(wx.wxHORIZONTAL, panel, "Days")
    sizer:Add(daysizer, 0, wx.wxEXPAND)
    local days = {}
    local daygrid = wx.wxGridSizer(1, 0, 5, 5)
    daysizer:Add(daygrid, 1, wx.wxEXPAND + wx.wxALL, 5)
    for i, label in ipairs({'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'}) do
        local ctrl = wx.wxCheckBox(panel, wx.wxID_ANY, label)
        ctrl.Value = puzzle.days[i]
        table.insert(days, ctrl)
        daygrid:Add(ctrl, 1, wx.wxEXPAND)
    end

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
    sizer:Add(panel, 1, wx.wxEXPAND + wx.wxALL, 5)
    sizer:Add(dlg:CreateButtonSizer(wx.wxOK + wx.wxCANCEL), 0, wx.wxALL, 5)
    dlg:Fit()
    dlg:Center()

    if dlg:ShowModal() == wx.wxID_OK then
        panel:apply()
        return puzzle
    end
end

-- ----------------------------------------------------------------------------
-- Config dialog
-- ----------------------------------------------------------------------------

local function get_download_fields(parent, puzzle)
    -- If a source has options, add them to a 'fields' table (see NYT Premium
    -- for example)
    local function field_key(name)
        return name:gsub('%s', ''):lower()
    end

    local fields = {}
    for _, name in ipairs(puzzle.fields or {}) do
        table.insert(fields, name)
        fields[name] = true
    end

    local scroller = wx.wxScrolledWindow(parent, wx.wxID_ANY)
    scroller:SetWindowStyle(wx.wxBORDER_DOUBLE)
    scroller:SetScrollRate(10, 10)

    -- This panel preserves tab behavior
    local psizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    local panel = wx.wxPanel(scroller, wx.wxID_ANY)
    psizer:Add(panel, 1, wx.wxEXPAND)
    scroller:SetSizer(psizer)

    local border = wx.wxBoxSizer(wx.wxVERTICAL)
    panel:SetSizer(border)

    local enabled = wx.wxCheckBox(panel, wx.wxID_ANY, "Enabled")
    enabled.Value = not download.disabled[puzzle.name]
    border:Add(enabled, 0, wx.wxEXPAND + wx.wxLEFT + wx.wxRIGHT + wx.wxTOP, 5)

    local sizer = wx.wxFlexGridSizer(0, 2, 5,5)
    border:Add(sizer, 1, wx.wxEXPAND + wx.wxALL, 5)
    sizer:AddGrowableCol(1)

    for _, name in ipairs(fields) do
        sizer:Add(wx.wxStaticText(panel, wx.wxID_ANY, name), 0, wx.wxALIGN_CENTER_VERTICAL)
        fields[name] = wx.wxTextCtrl(panel, wx.wxID_ANY, puzzle[field_key(name)] or '')
        sizer:Add(fields[name], 1, wx.wxEXPAND)
    end

    function scroller:apply()
        for _, name in ipairs(fields) do
            local ctrl = fields[name]
            puzzle[field_key(name)] = ctrl.Value
        end
        download.disabled[puzzle.name] = not enabled.Value
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
    local srcsizer = wx.wxStaticBoxSizer(wx.wxHORIZONTAL, panel, "Sources")
    sizer:Add(srcsizer, 1, wx.wxEXPAND + wx.wxTOP, 5)
    local puzzle_list = wx.wxListBox(panel, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxSize(-1, 100))
    srcsizer:Add(puzzle_list, 0, wx.wxEXPAND)

    local puzzles = deepcopy(download.puzzles)
    local names = {}
    for _, puzzle in ipairs(puzzles) do
        table.insert(names, puzzle.name)
    end
    puzzle_list:Set(names)

    local text = wx.wxStaticText(panel, wx.wxID_ANY, "Select a Puzzle", wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxALIGN_CENTER)
    srcsizer:Add(text, 1, wx.wxALIGN_CENTER)

    -- Details panel
    local details

    puzzle_list:Connect(wx.wxEVT_COMMAND_LISTBOX_SELECTED, function(evt)
        if details then
            details:apply()
            srcsizer:Detach(2)
            details:Destroy()
        end
        if puzzle_list.Selection == -1 then
            srcsizer:Show(1, true)
            text.Label = "Select a Puzzle"
        else
            srcsizer:Show(1, false)
            details = get_download_options_panel(panel, puzzles[puzzle_list.Selection + 1])
            srcsizer:Add(details, 1, wx.wxEXPAND + wx.wxLEFT, 5)
            srcsizer:Show(2, true)
        end
        srcsizer:Layout()
        evt:Skip()
    end)

    puzzle_list:Connect(wx.wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, function(evt)
        evt:Skip()
        if puzzle_list.Selection ~= -1 then
            show_advanced_options_dialog(panel, puzzles[puzzle_list.Selection + 1])
        end
    end)

    panel.MinSize = wx.wxSize(500, -1)

    function panel:apply()
        download.puzzle_directory = puzzle_directory.Path
        download.separate_directories = separate_directories.Selection == 1
        if details then details:apply() end
        clear(download.puzzles)
        for _, puzzle in ipairs(puzzles) do
            table.insert(download.puzzles, puzzle)
        end
    end

    return panel
end


function download.get_config_dialog(parent, id)
    xword.showerrors = false
    local dlg = wx.wxDialog(parent or xword.frame, id or wx.wxID_ANY, "Download Config")

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    dlg:SetSizer(sizer)
    local panel = download.get_config_panel(dlg)
    sizer:Add(panel, 1, wx.wxEXPAND + wx.wxALL, 5)
    sizer:Add(dlg:CreateButtonSizer(wx.wxOK + wx.wxCANCEL), 0, wx.wxALL, 5)
    dlg:Fit()

    function dlg:apply()
        panel:apply()
    end

    return dlg
end

function download.show_config_dialog(parent, id)
    local dlg = download.get_config_dialog(parent, id)
    dlg:Center()
    if dlg:ShowModal() == wx.wxID_OK then
        dlg:apply()
    end
end