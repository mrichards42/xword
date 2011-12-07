local deepcopy = require 'pl.tablex'.deepcopy

download.puzzle_directory = xword.userdatadir.."\\puzzles"
download.date_format = '%Y%m%d'
download.separate_directories = true

-- Load the config
local success, config = pcall(dofile, xword.configdir..'/download/config.lua')
if success and config then
    for _, name in ipairs({'puzzle_directory', 'date_format', 'separate_directories'}) do
        if config[name] ~= nil then
            download[name] = config[name]
        end
    end

    -- Remove download sources by name
    for _, name in ipairs(config.remove or {}) do
        for i, puzzle in ipairs(download.puzzles) do
            if puzzle.name == name then
                table.remove(download.puzzles, i)
                break
            end
        end
    end
    -- Add download sources
    for _, puzzle in ipairs(config.add or {}) do
        table.insert(download.puzzles, puzzle)
    end
end

-- Config gui

function download.get_puzzle_config_panel(parent)
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)

    local sizer1 = wx.wxBoxSizer(wx.wxHORIZONTAL)
    sizer:Add(sizer1, 0, wx.wxEXPAND + wx.wxBOTTOM, 5)

    local name = wx.wxTextCtrl(parent, wx.wxID_ANY)
    sizer1:Add(wx.wxStaticText(parent, wx.wxID_ANY, "Name:"), 0, wx.wxALIGN_CENTER)
    sizer1:Add(name, 1, wx.wxEXPAND + wx.wxLEFT, 5)

    local sizer2 = wx.wxBoxSizer(wx.wxHORIZONTAL)
    sizer:Add(sizer2, 0, wx.wxEXPAND + wx.wxBOTTOM, 5)

    local url = wx.wxTextCtrl(parent, wx.wxID_ANY)
    sizer2:Add(wx.wxStaticText(parent, wx.wxID_ANY, "URL:"), 0, wx.wxALIGN_CENTER)
    sizer2:Add(url, 1, wx.wxEXPAND + wx.wxLEFT, 5)

    local sizer3 = wx.wxBoxSizer(wx.wxHORIZONTAL)
    sizer:Add(sizer3, 0, wx.wxEXPAND + wx.wxBOTTOM, 5)

    local filename = wx.wxTextCtrl(parent, wx.wxID_ANY, "")
    sizer3:Add(wx.wxStaticText(parent, wx.wxID_ANY, "Local filename:"), 0, wx.wxALIGN_CENTER)
    sizer3:Add(filename, 1, wx.wxEXPAND + wx.wxLEFT, 5)

    local daysizer = wx.wxStaticBoxSizer(wx.wxHORIZONTAL, parent, "Days")
    sizer:Add(daysizer, 0, wx.wxEXPAND)
    local days = {}
    for i, label in ipairs({'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'}) do
        local ctrl = wx.wxCheckBox(parent, wx.wxID_ANY, label)
        table.insert(days, ctrl)
        daysizer:Add(ctrl)
    end

    function sizer:set_puzzle(puzzle)
        name.Value = puzzle.name
        filename.Value = puzzle.filename
        url.Value = puzzle.url
        for i, value in ipairs(puzzle.days) do
            days[i].Value = value
        end
    end
    return sizer
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
    local list = wx.wxCheckListBox(panel, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxSize(-1, 100))
    srcsizer:Add(list, 0, wx.wxEXPAND)

    local puzzles = deepcopy(download.puzzles)
    local names = {}
    for _, puzzle in ipairs(puzzles) do
        table.insert(names, puzzle.name)
    end
    list:Set(names)

    local puzzle_config = download.get_puzzle_config_panel(panel)
    srcsizer:Add(puzzle_config, 0, wx.wxEXPAND + wx.wxALL, 5)

    local text = wx.wxStaticText(panel, wx.wxID_ANY, "Select a Puzzle", wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxALIGN_CENTER)
    srcsizer:Add(text, 1, wx.wxALIGN_CENTER)

    srcsizer:Show(1, true)
    srcsizer:Show(2, false)
    panel:Fit()
    panel.MinSize = panel.Size
    srcsizer:Show(1, false)
    srcsizer:Show(2, true)
    panel:Layout()

    list:Connect(wx.wxEVT_COMMAND_LISTBOX_SELECTED, function(evt)
        if list.Selection == -1 then
            srcsizer:Show(1, false)
            srcsizer:Show(2, true)
        else
            srcsizer:Show(1, true)
            srcsizer:Show(2, false)
            puzzle_config:set_puzzle(puzzles[list.Selection + 1])
        end
        srcsizer:Layout()
        evt:Skip()
    end)

    return panel
end


function download.get_config_dialog(parent, id)
    local dlg = wx.wxDialog(parent or xword.frame, id or wx.wxID_ANY, "Download Config")

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    dlg:SetSizer(sizer)
    sizer:Add(download.get_config_panel(dlg), 1, wx.wxEXPAND + wx.wxALL, 5)
    dlg:Fit()

    return dlg
end
